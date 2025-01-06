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

string deleteUser(const char* displayName) {
    string user_dn = "CN=" + string(displayName) + "," + user_base_dn;
    cout << "Attempting to delete user with DN: " << user_dn << endl;

    rc = ldap_delete_ext_s(ld, user_dn.c_str(), NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        cerr << "Failed to delete user: " << ldap_err2string(rc) << endl;
        return "Failed to delete user: " + string(ldap_err2string(rc));
    } else {
        cout << "User deleted successfully from AD: " << displayName << endl;
        return "User deleted successfully";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <displayName>" << endl;
        return EXIT_FAILURE;
    }

    const char* displayName = argv[1];

    ldapBind();
    string result = deleteUser(displayName);
    cout << result << endl;
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return 0;
}