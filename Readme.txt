
1 - start the LDAP 
    * cd agent
    * g++ -o agent agent.cpp -lldap -lcurl
    * ./agent

2 - start the server
    * cd backend
    * run tamcat

3 - start the client
    * cd frontend 
    * npm start (or) ember serve