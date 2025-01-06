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
}

string deleteComputer(const char* computer_cn) {
    string computer_dn = "CN=" + string(computer_cn) + "," + comp_base_dn;

    rc = ldap_delete_ext_s(ld, computer_dn.c_str(), NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        cerr << "Failed to delete computer: " << ldap_err2string(rc) << endl;
        return "Failed to delete computer: " + string(ldap_err2string(rc));
    } else {
        cout << "Computer deleted successfully from AD: " << computer_cn << endl;
        return "Computer deleted successfully";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <computerName>" << endl;
        return EXIT_FAILURE;
    }

    const char* computerName = argv[1];

    ldapBind();
    string result = deleteComputer(computerName);
    cout << result << endl;
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return 0;
}