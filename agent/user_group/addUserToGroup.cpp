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
    if(rc != LDAP_SUCCESS) {
        cerr << "Failed to initialize LDAP connection: " << ldap_err2string(rc) << endl;
        exit(EXIT_FAILURE);
    }
    int ldap_version = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);

    BerValue cred;
    cred.bv_val = (char*)password;
    cred.bv_len = strlen(password);

    rc = ldap_sasl_bind_s(ld, username, LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
    if(rc != LDAP_SUCCESS) {
        cerr << "LDAP bind failed: " << ldap_err2string(rc) << endl;
        ldap_unbind_ext_s(ld, NULL, NULL);
        exit(EXIT_FAILURE);
    }
}

bool userExists(const char* user_cn) {
    LDAPMessage* result;
    string user_dn = "CN=" + string(user_cn) + "," + user_base_dn;
    rc = ldap_search_ext_s(ld, user_dn.c_str(), LDAP_SCOPE_BASE, "(objectClass=user)", NULL, 0, NULL, NULL, NULL, 0, &result);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP search failed: " << ldap_err2string(rc) << endl;
        return false;
    }

    int count = ldap_count_entries(ld, result);
    ldap_msgfree(result);
    return (count > 0);
}

bool userExistsInGroup(const char* group_dn, const char* user_dn) {
    LDAPMessage* result;
    string filter = "(member=" + string(user_dn) + ")";
    rc = ldap_search_ext_s(ld, group_dn, LDAP_SCOPE_BASE, filter.c_str(), NULL, 0, NULL, NULL, NULL, 0, &result);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP search failed: " << ldap_err2string(rc) << endl;
        return false;
    }

    int count = ldap_count_entries(ld, result);
    ldap_msgfree(result);
    return (count > 0);
}
string addUserToGroup(const char* group_cn, const char* user_cn) {
    if (!userExists(user_cn)) {
        return "User not found in AD";
    }

    string group_dn = "CN=" + string(group_cn) + "," + user_base_dn;
    string user_dn = "CN=" + string(user_cn) + "," + user_base_dn;

    if (userExistsInGroup(group_dn.c_str(), user_dn.c_str())) {
        return "User already exists in the group";
    }

    LDAPMod add_member;
    LDAPMod* data[2];
    const char* memberValues[] = {user_dn.c_str(), NULL};
    add_member.mod_op = LDAP_MOD_ADD;
    add_member.mod_type = const_cast<char*>("member");
    add_member.mod_vals.modv_strvals = const_cast<char**>(memberValues);
    data[0] = &add_member;
    data[1] = NULL;

    rc = ldap_modify_ext_s(ld,group_dn.c_str(), data, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        cerr << "Failed to add user to group: " << ldap_err2string(rc) << endl;
        return "Failed to add user to group: " + string(ldap_err2string(rc));
    } 
    else {
        return "User added to group successfully";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <groupName> <userName>" << endl;
        return EXIT_FAILURE;
    }

    const char* groupName = argv[1];
    const char* userName = argv[2];

    ldapBind();
    string result = addUserToGroup(groupName, userName);
    cout << result << endl;
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return 0;
}