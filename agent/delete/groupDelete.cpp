#include <iostream>
#include <ldap.h>
#include <cstdlib>
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
    cout << "LDAP bind successful" << endl;
}

string deleteGroup(const char* group_cn) {
    string group_dn = "CN=" + string(group_cn) + "," + user_base_dn;
    cout << "Attempting to delete group with DN: " << group_dn << endl;

    rc = ldap_delete_ext_s(ld, group_dn.c_str(), NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        cerr << "Failed to delete group: " << ldap_err2string(rc) << endl;
        return "Failed to delete group: " + string(ldap_err2string(rc));
    } else {
        cout << "Group deleted successfully from AD: " << group_cn << endl;
        return "Group deleted successfully";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <groupName>" << endl;
        return EXIT_FAILURE;
    }

    const char* groupName = argv[1];

    ldapBind();
    string result = deleteGroup(groupName);
    cout << result << endl;
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return 0;
}