#pragma once


#include "toSetSockets.hpp"
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

class Server {
    int fd;
    std::map<std::string, std::string> directives;
};

class ConfigFile {

private:
    //std::ifstream *fd;
    std::vector<Server> servers;


public:
   

    ConfigFile();
    // bool openCheckFile (std::ifstream &fd);
    // void    readConfigFile(std::ifstream &fd);



    ConfigFile(ConfigFile &other);
    ConfigFile(std::ifstream &fd_opened);


    void fillmainContextServersVector(std::vector<int> &servers);
    void setDirectivesForEach(std::map<std::string, std::string> &mapServer, size_t i);


    //? Getters
    const std::vector<int>  getmainContextServers() const;
    const std::vector<int>  getServers() const;
    const int  getNumsOfServers() const;



    //? Setters
    void setSockets(std::vector<int> set);

    // void setConfigFileFd(std::ifstream &fd_opened);

    ~ConfigFile();

};
