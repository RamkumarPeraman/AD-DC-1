#include <iostream>
#include <ldap.h>
#include <cstdlib>
#include <cstring>
#include "../ldap_config.h"


using namespace std;

LDAP* ld; 
int rc;  

// const char* ldap_server = "ldap://10.94.75.248";
// const char* username = "CN=Administrator,CN=Users,DC=zoho,DC=com";
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

void createGroup(const char* group_cn, const char* group_description, const char* group_mail) {
    LDAPMod objectClass, cn, description, mail;
    LDAPMod* data[5];

    const char* objectClassValues[] = { "top", "group", NULL };
    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = const_cast<char*>("objectClass");
    objectClass.mod_vals.modv_strvals = const_cast<char**>(objectClassValues);

    // Set the cn (common name) attribute
    const char* cnValues[] = { group_cn, NULL };
    cn.mod_op = LDAP_MOD_ADD;
    cn.mod_type = const_cast<char*>("cn");
    cn.mod_vals.modv_strvals = const_cast<char**>(cnValues);

    // Set the description attribute
    const char* descriptionValues[] = { group_description, NULL };
    description.mod_op = LDAP_MOD_ADD;
    description.mod_type = const_cast<char*>("description");
    description.mod_vals.modv_strvals = const_cast<char**>(descriptionValues);

    // Set the mail attribute   
    const char* mailValues[] = { group_mail, NULL };
    mail.mod_op = LDAP_MOD_ADD;
    mail.mod_type = const_cast<char*>("mail");
    mail.mod_vals.modv_strvals = const_cast<char**>(mailValues);
    data[0] = &objectClass;
    data[1] = &cn;
    data[2] = &description;
    data[3] = &mail;
    data[4] = NULL;
    string group_dn = "CN=" + string(group_cn) + "," + user_base_dn;
    rc = ldap_add_ext_s(ld, group_dn.c_str(), data, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        cerr << "Failed to create group: " << ldap_err2string(rc) << endl;
    } else {
        cout << "Group created successfully: " << group_cn << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <group_name> <description> <mail>" << endl;
        return EXIT_FAILURE;
    }

    const char* groupName = argv[1];
    const char* groupDescription = argv[2];
    const char* groupMail = argv[3];
    ldapBind();
    createGroup(groupName, groupDescription, groupMail);
    ldap_unbind_ext_s(ld, nullptr, nullptr);
}