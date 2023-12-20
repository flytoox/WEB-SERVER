#pragma once


#include <string>
#include <iostream>
#include <map>
#include <stack>
#include <netinet/in.h>
#include <vector>
#include <string.h>
#include <sys/socket.h>
#include <fstream>
#include <sys/_endian.h>
#include <sstream>


class Server {

public:


    int socketD;
    struct sockaddr_in serverAddress;
    std::map<std::string, std::string> directives;
    std::string serverName;
    std::vector<std::map<std::string, std::string> > locationsBlock;
    Server();


    void bindSockets();
    void listenToIncomingConxs();

    int getSocketDescriptor() const;
    std::string getServerName() const;  
    std::map<std::string, std::string> const& getdirectives(void);
    std::vector<std::map<std::string, std::string> > const& getlocationsBlock(void);



    void setServerAddress(struct sockaddr_in &eachServerAddress);
    void setSocketDescriptor(int &socketDescriptor);
    void setDirectives(std::map<std::string, std::string> &newDirectives);
    void setLocationBlock(std::vector<std::map<std::string, std::string> > &newLocDirectives);

    
    ~Server();

};
