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

bool computerExists(const char* name) {
    LDAPMessage* result;
    string filter = "(cn=" + string(name) + ")";
    rc = ldap_search_ext_s(ld, comp_base_dn, LDAP_SCOPE_SUBTREE, filter.c_str(), NULL, 0, NULL, NULL, NULL, 0, &result);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP search failed: " << ldap_err2string(rc) << endl;
        return false;
    }

    int count = ldap_count_entries(ld, result);
    ldap_msgfree(result);
    return (count > 0);
}

void createComputer(const char* name, const char* description, const char* location) {
    if (computerExists(name)) {
        cout << "Computer already exists with name: " << name << endl;
        return;
    }

    LDAPMod objectClass, cn, descriptionAttr, locationAttr;
    LDAPMod* data[5];

    const char* objectClassValues[] = { "top", "person", "organizationalPerson", "computer", NULL };
    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = const_cast<char*>("objectClass");
    objectClass.mod_vals.modv_strvals = const_cast<char**>(objectClassValues);

    const char* cnValues[] = { name, NULL };
    cn.mod_op = LDAP_MOD_ADD;
    cn.mod_type = const_cast<char*>("cn");
    cn.mod_vals.modv_strvals = const_cast<char**>(cnValues);

    const char* descriptionValues[] = { description, NULL };
    descriptionAttr.mod_op = LDAP_MOD_ADD;
    descriptionAttr.mod_type = const_cast<char*>("description");
    descriptionAttr.mod_vals.modv_strvals = const_cast<char**>(descriptionValues);

    const char* locationValues[] = { location, NULL };
    locationAttr.mod_op = LDAP_MOD_ADD;
    locationAttr.mod_type = const_cast<char*>("l");
    locationAttr.mod_vals.modv_strvals = const_cast<char**>(locationValues);

    data[0] = &objectClass;
    data[1] = &cn;
    data[2] = &descriptionAttr;
    data[3] = &locationAttr;
    data[4] = NULL;

    string computer_dn = "CN=" + string(name) + "," + comp_base_dn;
    rc = ldap_add_ext_s(ld, computer_dn.c_str(), data, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        cerr << "Failed to create computer: " << ldap_err2string(rc) << endl;
    } else {
        cout << "Computer created successfully: " << name << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <name> <description> <location>" << endl;
        return EXIT_FAILURE;
    }
    const char* name = argv[1];
    const char* description = argv[2];
    const char* location = argv[3];

    ldapBind();
    createComputer(name, description, location);
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return 0;
}