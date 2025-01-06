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

bool userExists(const char* displayname) {
    LDAPMessage* result;
    string filter = "(displayName=" + string(displayname) + ")";
    rc = ldap_search_ext_s(ld, user_base_dn, LDAP_SCOPE_SUBTREE, filter.c_str(), NULL, 0, NULL, NULL, NULL, 0, &result);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP search failed: " << ldap_err2string(rc) << endl;
        return false;
    }

    int count = ldap_count_entries(ld, result);
    ldap_msgfree(result);
    cout << "User exists check completed. Count: " << count << endl;
    return (count > 0);
}

void createUser(const char* firstName, const char* lastName, const char* mail, const char* phnnumber, const char* description, const char* displayname) {
    if (userExists(displayname)) {
        cout << "User already exists with display name: " << displayname << endl;
        return;
    }

    LDAPMod objectClass, cn, sn, givenName, userPrincipalName, userDescription, mailAttr, telephoneNumber, displayName;
    LDAPMod* data[10];

    const char* objectClassValues[] = { "top", "person", "organizationalPerson", "user", NULL };
    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = const_cast<char*>("objectClass");
    objectClass.mod_vals.modv_strvals = const_cast<char**>(objectClassValues);

    const char* cnValues[] = { displayname, NULL };
    cn.mod_op = LDAP_MOD_ADD;
    cn.mod_type = const_cast<char*>("cn");
    cn.mod_vals.modv_strvals = const_cast<char**>(cnValues);

    const char* snValues[] = { lastName, NULL };
    sn.mod_op = LDAP_MOD_ADD;
    sn.mod_type = const_cast<char*>("sn");
    sn.mod_vals.modv_strvals = const_cast<char**>(snValues);

    const char* givenNameValues[] = { firstName, NULL };
    givenName.mod_op = LDAP_MOD_ADD;
    givenName.mod_type = const_cast<char*>("givenName");
    givenName.mod_vals.modv_strvals = const_cast<char**>(givenNameValues);

    const char* userPrincipalNameValues[] = { mail, NULL };
    userPrincipalName.mod_op = LDAP_MOD_ADD;
    userPrincipalName.mod_type = const_cast<char*>("userPrincipalName");
    userPrincipalName.mod_vals.modv_strvals = const_cast<char**>(userPrincipalNameValues);

    const char* userDescriptionValues[] = { description, NULL };
    userDescription.mod_op = LDAP_MOD_ADD;
    userDescription.mod_type = const_cast<char*>("description");
    userDescription.mod_vals.modv_strvals = const_cast<char**>(userDescriptionValues);

    const char* mailValues[] = { mail, NULL };
    mailAttr.mod_op = LDAP_MOD_ADD;
    mailAttr.mod_type = const_cast<char*>("mail");
    mailAttr.mod_vals.modv_strvals = const_cast<char**>(mailValues);

    const char* telephoneNumberValues[] = { phnnumber, NULL };
    telephoneNumber.mod_op = LDAP_MOD_ADD;
    telephoneNumber.mod_type = const_cast<char*>("telephoneNumber");
    telephoneNumber.mod_vals.modv_strvals = const_cast<char**>(telephoneNumberValues);

    const char* displayNameValues[] = { displayname, NULL };
    displayName.mod_op = LDAP_MOD_ADD;
    displayName.mod_type = const_cast<char*>("displayName");
    displayName.mod_vals.modv_strvals = const_cast<char**>(displayNameValues);

    data[0] = &objectClass;
    data[1] = &cn;
    data[2] = &sn;
    data[3] = &givenName;
    data[4] = &userPrincipalName;
    data[5] = &userDescription;
    data[6] = &mailAttr;
    data[7] = &telephoneNumber;
    data[8] = &displayName;
    data[9] = NULL;

    string user_dn = "CN=" + string(displayname) + "," + user_base_dn;
    rc = ldap_add_ext_s(ld, user_dn.c_str(), data, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        cerr << "Failed to create user: " << ldap_err2string(rc) << endl;
    } else {
        cout << "User created successfully: " << displayname << endl;
    }
}
int main(int argc, char* argv[]) {
    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " <firstName> <lastName> <mail> <phnnumber> <description> <displayname>" << endl;
        return EXIT_FAILURE;
    }

    const char* firstName = argv[1];
    const char* lastName = argv[2];
    const char* mail = argv[3];
    const char* phnnumber = argv[4];
    const char* description = argv[5];
    const char* displayname = argv[6];

    cout << "Starting LDAP operations..." << endl;
    ldapBind();
    createUser(firstName, lastName, mail, phnnumber, description, displayname);
    ldap_unbind_ext_s(ld, nullptr, nullptr);
    cout << "LDAP operations completed." << endl;

    return 0;
}