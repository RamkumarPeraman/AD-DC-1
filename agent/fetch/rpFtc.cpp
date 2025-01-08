#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <ldap.h>
#include <cstring>
#include "../ldap_config.h"

using namespace std;

LDAP* ld;
int rc;

void ldapBind() {
    rc = ldap_initialize(&ld, ldap_server);
    if (rc != LDAP_SUCCESS) {
        cerr << "Failed to initialize LDAP connection: " << ldap_err2string(rc) << endl;
        exit(EXIT_FAILURE);
    }
    int ldap_version = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
    BerValue cred;
    cred.bv_val = (char*)password;
    cred.bv_len = strlen(password);

    rc = ldap_sasl_bind_s(ld, username, LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP bind failed: " << ldap_err2string(rc) << endl;
        ldap_unbind_ext_s(ld, NULL, NULL);
        exit(EXIT_FAILURE);
    }
}
map<string, string> fetchAttributes(const string& groupName) {
    map<string, string> attributes;
    string filter = "(cn=" + groupName + ")";
    LDAPMessage* result = nullptr;
    const char* attrs[] = {"mail", "description", "telephoneNumber","uSNChanged", NULL};
    rc = ldap_search_ext_s(ld, user_base_dn, LDAP_SCOPE_SUBTREE, filter.c_str(), const_cast<char**>(attrs), 0, NULL, NULL, NULL, 0, &result);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP search failed: " << ldap_err2string(rc) << endl;
        ldap_msgfree(result);
        return attributes;
    }
    LDAPMessage* entry = ldap_first_entry(ld, result);
    if (entry != NULL) {
        BerElement* ber;
        char* attribute = ldap_first_attribute(ld, entry, &ber);
        while (attribute != NULL) {
            struct berval** values = ldap_get_values_len(ld, entry, attribute);
            if (values != NULL) {
                attributes[attribute] = values[0]->bv_val;
                ldap_value_free_len(values);
            }
            ldap_memfree(attribute);
            attribute = ldap_next_attribute(ld, entry, ber);
        }
        if (ber != NULL) {
            ber_free(ber, 0);
        }
    }
    ldap_msgfree(result);
    return attributes;
}

string compareAttributes(const string& groupName, const map<string, string>& previousState) {
    auto currentAttributes = fetchAttributes(groupName);
    for (const auto& [key, value] : currentAttributes) {
        if (previousState.find(key) == previousState.end() || previousState.at(key) != value) {
            return "{\"name\": \"" + groupName + "\", \"lastModifiedField\": \"" + key + "\", \"value\": \"" + value + "\", \"uSNChanged\": \"" + currentAttributes["uSNChanged"] + "\"}";
        }
    }
    return "{}";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <groupName>" << endl;
        return 1;
    }
    string groupName = argv[1];
    ifstream inputFile("state.txt");
    map<string, string> previousState;
    string key, value;
    while (inputFile >> key >> value) {
        previousState[key] = value;
    }
    inputFile.close();
    ldapBind();
    string result = compareAttributes(groupName, previousState);
    cout << result << endl;
    ofstream outputFile("state.txt");
    auto currentState = fetchAttributes(groupName);
    for (const auto& [key, value] : currentState) {
        outputFile << key << " " << value << endl;
    }
    outputFile.close();

    ldap_unbind_ext_s(ld, nullptr, nullptr);
    return 0;
}
