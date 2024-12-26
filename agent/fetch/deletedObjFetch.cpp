#include <iostream>
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

string getDeletedObjectDetails(const string& objectName) {
    string filter = "(&(isDeleted=TRUE)(|(cn=" + objectName + ")(displayName=" + objectName + ")))";
    
    // Apply the deleted object control
    LDAPControl deleted_control = {(char*)"1.2.840.113556.1.4.417", {0, NULL}, 1};
    LDAPControl* server_controls[] = {&deleted_control, NULL};
    LDAPMessage* result = nullptr;

    // Execute the search
    rc = ldap_search_ext_s(ld, dlt_base_dn, LDAP_SCOPE_SUBTREE, filter.c_str(), nullptr, 0, server_controls, nullptr, nullptr, LDAP_NO_LIMIT, &result);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP search failed: " << ldap_err2string(rc) << endl;
        ldap_msgfree(result);
        return "{}";
    }

    LDAPMessage* entry = ldap_first_entry(ld, result);
    if (entry == NULL) {
        cerr << "No entry found for deleted object: " << objectName << endl;
        ldap_msgfree(result);
        return "{}";
    }

    string description, mail, objectClass;
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
            if (strcmp(attribute, "objectClass") == 0) {
                objectClass = values[0]->bv_val;
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
    return "{\"name\": \"" + objectName + "\", \"description\": \"" + description + "\", \"mail\": \"" + mail + "\", \"objectClass\": \"" + objectClass + "\"}";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <objectName>" << endl;
        return 1;
    }

    string objectName = argv[1];
    ldapBind();
    string objectDetails = getDeletedObjectDetails(objectName);
    cout << objectDetails << endl;

    ldap_unbind_ext_s(ld, nullptr, nullptr);
    return 0;
}
