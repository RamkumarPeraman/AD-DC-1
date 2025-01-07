#include <iostream>
#include <string>
#include <ldap.h>
#include <cstring>
#include <ctime>
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

string convertToIndianFormat(const string& ldapTime) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    strptime(ldapTime.c_str(), "%Y%m%d%H%M%SZ", &tm);

    time_t rawTime = mktime(&tm) + 19800;
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&rawTime));
    return string(buffer);
}

string getGroupDetails(const string& groupName) {
    string filter = "(cn=" + groupName + ")";
    LDAPMessage* result = nullptr;
    const char* attrs[] = {"description", "mail", "whenCreated", "whenChanged", NULL}; // Specify attributes to fetch
    rc = ldap_search_ext_s(ld, user_base_dn, LDAP_SCOPE_SUBTREE, filter.c_str(), const_cast<char**>(attrs), 0, NULL, NULL, NULL, 0, &result);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP search failed: " << ldap_err2string(rc) << endl;
        ldap_msgfree(result);
        return "{}";
    }
    LDAPMessage* entry = ldap_first_entry(ld, result);
    if (entry == NULL) {
        cerr << "No entry found for group: " << groupName << endl;
        ldap_msgfree(result);
        return "{}";
    }

    string description, mail, whenCreated, whenChanged;
    BerElement* ber;
    char* attribute = ldap_first_attribute(ld, entry, &ber);
    while (attribute != NULL) {
        struct berval** values = ldap_get_values_len(ld, entry, attribute);
        if (values != NULL) {
            if (strcmp(attribute, "description") == 0) {
                description = values[0]->bv_val;
            }
            if (strcmp(attribute, "mail") == 0) {
                mail = values[0]->bv_val;
            }
            if (strcmp(attribute, "whenCreated") == 0) {
                whenCreated = convertToIndianFormat(values[0]->bv_val);
            }
            if (strcmp(attribute, "whenChanged") == 0) {
                whenChanged = convertToIndianFormat(values[0]->bv_val);
            }
            ldap_value_free_len(values);
        }
        ldap_memfree(attribute);
        attribute = ldap_next_attribute(ld, entry, ber);
    }
    if (ber != NULL) {
        ber_free(ber, 0);
    }
    ldap_msgfree(result);
    return "{\"name\": \"" + groupName + "\", \"description\": \"" + description + "\", \"mail\": \"" + mail + "\", \"whenCreated\": \"" + whenCreated + "\", \"whenChanged\": \"" + whenChanged + "\"}";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <groupName>" << endl;
        return 1;
    }
    string groupName = argv[1];
    ldapBind();
    string groupDetails = getGroupDetails(groupName);
    cout << groupDetails << endl;
    
    ldap_unbind_ext_s(ld, nullptr, nullptr);
    return 0;
}
