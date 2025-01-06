#include <iostream>
#include <string>
#include <ldap.h>
#include <ctime>
#include <unistd.h>
#include <curl/curl.h>
#include <unordered_set>
#include <vector>
#include <cstdlib>
#include <cstring>
#include "ldap_config.h"
using namespace std;

string URL = "http://localhost:8080/backend_war_exploded";

vector<string> ou_names;
time_t lastCheckedTime = 0;
char* attribute; 
BerElement* ber;  
LDAPMessage* result, *entry;  
struct berval** values; 
unordered_set<string> processedEntries;
LDAP* ld;
int rc;
bool initialFetch = true;
bool servletSend = false;
string timeFilter;
string combinedFilter;
string  userData,groupData,computerData,ouData,deletedObjectData;

void sendDataToServlet(const string& servletUrl, const string& postData) {
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, servletUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            cerr << "CURL failed: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

}
void ldapBind(){
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
string getLDAPTimeString(time_t rawtime) {
    struct tm* timeinfo;
    char buffer[20]; 
    timeinfo = gmtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S.0Z", timeinfo);
    return string(buffer);
}
// Function to parse the date and time string
bool parseTime(const char* str, struct tm& tm) {
    return sscanf(str, "%4d%2d%2d%2d%2d%2d",
                  &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                  &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6;
}

// Function to get the last modification time
time_t getLastModificationTime(LDAP* ld, LDAPMessage* entry) {
    BerElement* ber;
    struct berval** values = ldap_get_values_len(ld, entry, "modifyTimestamp");
    if (values == NULL) {
        return 0; // Handle error: No modification timestamp found
    }

    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    tm.tm_isdst = -1; // Not using daylight saving time

    if (parseTime(values[0]->bv_val, tm)) {
        // Adjust year and month values
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        time_t result = mktime(&tm);
        ldap_value_free_len(values);
        return result;
    } else {
        ldap_value_free_len(values);
        return 0; // Handle error: Failed to parse time
    }
}
void dataTraverse(const char* base_dn, const char* filter, const char* attributes[], void (*processEntry)(LDAP* ld, LDAPMessage* entry)) {
    LDAPMessage* result = nullptr;
    int rc = ldap_search_ext_s(ld, base_dn, LDAP_SCOPE_SUBTREE, filter, const_cast<char**>(attributes), 0, NULL, NULL, NULL, 0, &result);
    if (rc == LDAP_SUCCESS) {    
        for (LDAPMessage* entry = ldap_first_entry(ld, result); entry != NULL; entry = ldap_next_entry(ld, entry)) {
            time_t entryLastModTime = getLastModificationTime(ld, entry);
            string dn = ldap_get_dn(ld, entry);
            if (initialFetch || (entryLastModTime > lastCheckedTime && processedEntries.find(dn) == processedEntries.end())) {
                processEntry(ld, entry);
                processedEntries.insert(dn);
            }
        }
    }
    ldap_msgfree(result);
}
void dataAddToVal(struct berval** values, string& val){// Data add to values
    if(values != NULL) {
        val = values[0]->bv_val;
        ldap_value_free_len(values);
    }
}
void processUserEntry(LDAP* ld, LDAPMessage* entry){// user entery
    string userName, description;
    struct berval** values = ldap_get_values_len(ld, entry, "displayName");
    dataAddToVal(values,userName);
    values = ldap_get_values_len(ld, entry, "description");
    dataAddToVal(values,description);

    if (!userName.empty() && !description.empty()) {
        string userPostData = "{\"userName\":\"" + userName + "\", \"description\":\"" + description +"\"},";
        sendDataToServlet(URL+"/UserDataServlet", userPostData);
        userData += userPostData;
        servletSend = false;                
        // cout << "User data sent to UserDataServlet: " << givenName << ", " << sn << ", " << description << endl;
    }
}
void fetchUserData(const char* base_dn, const char* filter){// Fetch user data
    if (!initialFetch) {
        timeFilter = "(whenChanged>=" + getLDAPTimeString(lastCheckedTime) + ")";
        combinedFilter = "(&" + string(filter) + timeFilter + ")";
    } else {
        combinedFilter = string(filter);
    }
    const char* user_attributes[] = {"displayName", "description", "whenChanged", NULL};
    userData += "[";
    dataTraverse(base_dn, combinedFilter.c_str(), user_attributes, processUserEntry);
    if(!userData.empty() && !servletSend){
        if (userData.back() == ',') {
            userData.pop_back();
        }
        userData += "]";
        string finalData = "{\"type\": \"User\", \"Users\": " + userData + "}";
        sendDataToServlet(URL+"/UserDataServlet", finalData);
        // cout << "-------------------------------------------------------"<< endl;
        // cout << "All User data sent to UserDataServlet: " << endl << finalData << endl;
        servletSend = true;
        userData ="";
    }
}
void processGroupEntry(LDAP* ld, LDAPMessage* entry) {
    string groupName, groupDescription,mail;
    struct berval** values = ldap_get_values_len(ld, entry, "cn");
    dataAddToVal(values, groupName);
    values = ldap_get_values_len(ld, entry, "description");
    dataAddToVal(values, groupDescription);
    values = ldap_get_values_len(ld, entry, "mail");
    dataAddToVal(values, mail);
    time_t groupLastModTime = getLastModificationTime(ld, entry);
    string dn = ldap_get_dn(ld, entry);
    if (initialFetch || (groupLastModTime > lastCheckedTime && processedEntries.find(dn) == processedEntries.end())) {
        if (!groupName.empty() && !groupDescription.empty()) {
            mail = !mail.empty() ? mail : "No mail found";
            string groupPostData = "{\"type\":\"group\",\"groupName\":\"" + groupName + "\",\"description\":\"" + groupDescription + "\",\"mail\":\"" + mail + "\"}";

            // Send data to servlet
            // sendDataToServlet(URL + "/GroupDataServlet", groupPostData);
            sendDataToServlet("http://localhost:8080/backend_war_exploded/GroupDataServlet", groupPostData);
           
            // cout << groupPostData << endl;
            // cout << "Group data sent to GroupServlet: " << groupName << endl;

            processedEntries.insert(dn); // Add to processed entries to avoid duplicates
        }
    }
}
// Function to fetch group data based on a base DN
void fetchGroupData(const char* base_dn) {
    string combinedFilter;
    if (!initialFetch) {
        // Create a filter to get entries changed after lastCheckedTime
        string timeFilter = "(whenChanged>=" + getLDAPTimeString(lastCheckedTime) + ")";
        combinedFilter = "(&(objectClass=group)" + timeFilter + ")";
    } else {
        combinedFilter = "(objectClass=group)";
    }
    const char* group_attributes[] = {"cn", "description", "whenChanged","mail", NULL};
    dataTraverse(base_dn, combinedFilter.c_str(), group_attributes, processGroupEntry);
}
void processComputerEntry(LDAP* ld, LDAPMessage* entry) {
     string computerName, computerDescription;
    struct berval** values = ldap_get_values_len(ld, entry, "cn");
    dataAddToVal(values, computerName);
    values = ldap_get_values_len(ld, entry, "description");
    dataAddToVal(values, computerDescription);    
    if (!computerName.empty() && !computerDescription.empty()) {
         string computerPostData = "{\"computerName\":\"" + computerName + "\", \"description\":\"" + computerDescription + "\"},";
        computerData += computerPostData;
        servletSend = false;
    }
}
// Function to fetch computer data from LDAP
void fetchComputerData(const char* base_dn) {
     string combinedFilter;
    if (!initialFetch) {
         string timeFilter = "(whenChanged>=" + getLDAPTimeString(lastCheckedTime) + ")";
        combinedFilter = "(&(objectClass=computer)" + timeFilter + ")";
    } else {
        combinedFilter = "(objectClass=computer)";
    }
    const char* computer_attributes[] = {"cn", "description", "whenChanged", NULL};
        computerData += "[";
        dataTraverse(base_dn, combinedFilter.c_str(), computer_attributes, processComputerEntry);
        if (!computerData.empty() && !servletSend) {
        if (computerData.back() == ',') {
            computerData.pop_back();  
        }
        computerData += "]";
        string finalData = "{\"type\": \"computer\", \"computers\": " + computerData + "}";
        sendDataToServlet(URL + "/ComputerDataServlet", finalData);
        
        //  cout << "-------------------------------------------------------" <<  endl;
        //  cout << "Computer data sent to ComputerDataServlet: " <<  endl << finalData <<  endl;
        servletSend = true;
        computerData = "";
    }
}
void processOUEntry(LDAP* ld, LDAPMessage* entry) {
    string ouName, ouDescription,street, pobox, city, state, postalCode, country;
    struct berval** values = ldap_get_values_len(ld, entry, "ou");
    dataAddToVal(values, ouName);
    values = ldap_get_values_len(ld, entry, "description");
    dataAddToVal(values, ouDescription);
    values = ldap_get_values_len(ld, entry, "streetAddress");
    dataAddToVal(values, street);

    values = ldap_get_values_len(ld, entry, "postOfficeBox");
    dataAddToVal(values, pobox);

    values = ldap_get_values_len(ld, entry, "l");
    dataAddToVal(values, city);

    values = ldap_get_values_len(ld, entry, "st");
    dataAddToVal(values, state);

    values = ldap_get_values_len(ld, entry, "postalCode");
    dataAddToVal(values, postalCode);

    values = ldap_get_values_len(ld, entry, "co");
    dataAddToVal(values, country);

    time_t ouLastModTime = getLastModificationTime(ld, entry);
    string dn = ldap_get_dn(ld, entry);
    if (initialFetch || (ouLastModTime > lastCheckedTime && processedEntries.find(dn) == processedEntries.end())) {
        if (!ouName.empty() && !ouDescription.empty()) {
            // Construct JSON data as a string
            string address = "";
            if(!street.empty()){
                address += street +",";
            }
            if(!pobox.empty()){
                address += pobox +",";
            }
            if(!city.empty()){
                address += city +",";
            }
            if(!state.empty()){
                address += state +",";
            }
            if(!country.empty()){
                address += country +",";
            }
            if(!postalCode.empty()){
                address += postalCode;
            }
            else{
                address += "No address found";
            }

            string ouPostData = "{\"type\":\"organizationalUnit\",\"ouName\":\"" + ouName + "\",\"description\":\"" + ouDescription + "\",\"address\":\"" + address + "\"}";

            sendDataToServlet(URL + "/OUServlet", ouPostData);
            cout << "Organizational Unit data sent to OUServlet: " << ouName << ", " << ouDescription <<"," << address<< endl;

            processedEntries.insert(dn); 
        }
    }
}
void fetchOUData(const char* base_dn) {
    string combinedFilter;
    if (!initialFetch) {
        string timeFilter = "(whenChanged>=" + getLDAPTimeString(lastCheckedTime) + ")";
        combinedFilter = "(&(objectClass=organizationalUnit)" + timeFilter + ")";
    } else {
        combinedFilter = "(objectClass=organizationalUnit)";
    }
    const char* ou_attributes[] = {"ou", "description", "whenChanged", "streetAddress", "postOfficeBox", "l", "st", "postalCode", "co", NULL};
    dataTraverse(base_dn, combinedFilter.c_str(), ou_attributes, processOUEntry);
}
void processDeletedObjectEntry(LDAP* ld, LDAPMessage* entry) {
    string objectType, objectName, objectDescription;
    struct berval** values = ldap_get_values_len(ld, entry, "objectClass");

    if (values != nullptr) {
        for (int i = 0; values[i] != nullptr; ++i) {
            string objectClass = values[i]->bv_val;
            if (objectClass == "computer") {
                objectType = "computer";
                break;
            } else if (objectClass == "group") {
                objectType = "group";
                break;
            } else if (objectClass == "user") {
                objectType = "user";
                break;
            } else if (objectClass == "organizationalUnit") {
                objectType = "organizationalUnit";
                break;
            }
        }
        ldap_value_free_len(values);
    }

    values = ldap_get_values_len(ld, entry, "cn");
    dataAddToVal(values, objectName);
    values = ldap_get_values_len(ld, entry, "description");
    dataAddToVal(values, objectDescription);
    time_t dltLastModTime = getLastModificationTime(ld, entry);
    string dn = ldap_get_dn(ld, entry);
    if (initialFetch || (dltLastModTime > lastCheckedTime && processedEntries.find(dn) == processedEntries.end())) {
        if (!objectType.empty() && !objectName.empty() && !objectDescription.empty()) {
            // Trim the "DEL" marker and UUID from the objectName
            int len = objectName.size();
            if (len > 41) {  // Ensure the length is sufficient to trim
                string objName = objectName.substr(0, len - 41);
                objectName = objName;  // Update objectName to trimmed name
            }

            // Construct JSON data for a deleted object entry
            string deletedObjectPostData = "{\"type\":\"" + objectType + "\",";
            if (objectType == "user") {
                deletedObjectPostData += "\"userName\":\"" + objectName + "\",";
            } else if (objectType == "computer") {
                deletedObjectPostData += "\"computerName\":\"" + objectName + "\",";
            } else if (objectType == "group") {
                deletedObjectPostData += "\"groupName\":\"" + objectName + "\",";
            } else if (objectType == "organizationalUnit") {
                deletedObjectPostData += "\"ouName\":\"" + objectName + "\",";
            }
            deletedObjectPostData += "\"description\":\"" + objectDescription + "\"}";

            // Send data to servlet
            sendDataToServlet(URL + "/DeletedObjServlet", deletedObjectPostData);
            cout << "Deleted object data sent to DeletedObjServlet: " << objectType << ": " << objectName << ", " << objectDescription << endl;

            processedEntries.insert(dn); // Add to processed entries to avoid duplicates
        }
    }
}

// Function to fetch deleted objects based on a base DN
void fetchDeletedObjects(const char* base_dn) {
    string combinedFilter;
    if (!initialFetch) {
        // Create a filter to get entries changed after lastCheckedTime
        string timeFilter = "(whenChanged>=" + getLDAPTimeString(lastCheckedTime) + ")";
        combinedFilter = "(&(isDeleted=TRUE)" + timeFilter + ")";
    } else {
        combinedFilter = "(isDeleted=TRUE)";
    }

    LDAPControl deleted_control = {(char*)"1.2.840.113556.1.4.417", {0, nullptr}, 1};
    LDAPControl* server_controls[] = {&deleted_control, nullptr};
    LDAPMessage* result = nullptr;
    int rc = ldap_search_ext_s(ld, base_dn, LDAP_SCOPE_SUBTREE, combinedFilter.c_str(), nullptr, 0, server_controls, nullptr, nullptr, LDAP_NO_LIMIT, &result);

    if (rc != LDAP_SUCCESS) {
        cerr << "ldap_search_ext_s: " << ldap_err2string(rc) << endl;
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return;
    }

    for (LDAPMessage* entry = ldap_first_entry(ld, result); entry != nullptr; entry = ldap_next_entry(ld, entry)) {
        processDeletedObjectEntry(ld, entry);
    }
    ldap_msgfree(result);
}


void fetchFromUsers(const char* base_dn){//fetch the data from Users
        fetchUserData(base_dn, "(objectClass=user)"); 
        fetchGroupData(base_dn);
        fetchComputerData(base_dn);

}
void fetchFromComputers (const char* base_dn){// function to fetc the data from the computer
        fetchComputerData(base_dn);
        fetchUserData(base_dn, "(objectClass=user)"); 
        fetchGroupData(base_dn);
}
void fetchFromOU(const char* base_dn){// function to fetch data from OU's
        fetchOUData(base_dn);
        fetchUserData(base_dn, "(objectClass=user)"); 
        fetchComputerData(base_dn);
        fetchGroupData(base_dn);
}
void fetchOu(){
    const char* ou_filter = "(objectClass=organizationalUnit)";
    const char* ou_attributes[] = {"ou", NULL};
    const char* base_dn = "DC=zoho,DC=com";
    rc = ldap_search_ext_s(ld, base_dn, LDAP_SCOPE_SUBTREE, ou_filter, const_cast<char**>(ou_attributes), 0, NULL, NULL, NULL, 0, &result);
    if (rc != LDAP_SUCCESS) {
        cerr << "LDAP OU search failed: " << ldap_err2string(rc) << endl;
        ldap_unbind_ext_s(ld, NULL, NULL);
        return ;
    }
    for (entry = ldap_first_entry(ld, result); entry != NULL; entry = ldap_next_entry(ld, entry)) {
        string ouName;
        for (attribute = ldap_first_attribute(ld, entry, &ber); attribute != NULL; attribute = ldap_next_attribute(ld, entry, ber)) {
            if ((values = ldap_get_values_len(ld, entry, attribute)) != NULL) {
                if (strcmp(attribute, "ou") == 0) {
                    ouName = values[0]->bv_val;
                }
                ldap_value_free_len(values);
            }
            ldap_memfree(attribute);
        }
        if (!ouName.empty()) {
            ou_names.push_back("OU="+ouName+",DC=zoho,DC=com"); // storing the OU's in the array
        }
        if (ber != nullptr) {
            ber_free(ber, 0);
        }
    }
    ldap_msgfree(result);
}
int main(){
    ldapBind(); 
    fetchOu();
    while(true){
        fetchDeletedObjects(dlt_base_dn);
        fetchFromUsers(user_base_dn);
        fetchFromComputers(comp_base_dn);
        for(string ou_base_dn : ou_names){
            fetchFromOU(ou_base_dn.c_str());
        }
        lastCheckedTime = time(nullptr);
        initialFetch = false;        
        sleep(120);
    }
    ldap_unbind_ext_s(ld, nullptr, nullptr);
    return 0;
}
