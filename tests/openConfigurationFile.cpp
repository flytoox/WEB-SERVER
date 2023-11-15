#include "configurationFile.hpp"


// bool ConfigFile::openCheckFile (std::ifstream &fd) {
//     if ( ! fd.is_open() )
//         return (false);
//     return (true);
// }


// void ConfigFile::setConfigFileFd(std::ifstream &fd_opened) {
//     this->fd = &fd_opened;
// }


// void    ConfigFile::readConfigFile (std::ifstream &fd) {
//     std::string line;
//     bool braces_check = false;
//     std::map<std::string, std::string> map;
//     while (getline(fd, line)) {
//         if (line == "server ") {
//             continue ;
//             if (line == "{") { braces_check = true; continue ; }
//         } else if (line == "server {") {
//             continue ;
//         }
//         if ()
//     }
// }


ConfigFile::ConfigFile() {}

void ConfigFile::fillmainContextServersVector(std::vector<int> &servers) {

    for (size_t i = 0; i < 2; i++) {
        servers[i] = i;
    }

}

void ConfigFile::setDirectivesForEach(std::map<std::string, std::string> &mapServer, size_t i)
{


    if (i == 0) {
        mapServer["listen"] = "3333";
        mapServer["server_name"] = "localhost";
        mapServer["autoindex"] = "on";
        mapServer["root"] = "/Users/sizgunan";
        mapServer["client_max_body_size"] = "8000";
        mapServer["index"] = "index.html";
    }

    if (i == 1) {
        mapServer["listen"] = "8080";
        mapServer["server_name"] = "localhost";
        mapServer["autoindex"] = "on";
        mapServer["root"] = "/";
        mapServer["client_max_body_size"] = "1002020";
        mapServer["index"] = "index.html";
    }



}

const std::vector<int>  ConfigFile::getServers() const {
    return (this->mainContextServers);
}

const int  ConfigFile::getNumsOfServers() const {
    return (this->mainContextServers.size());
}

const std::vector<int>  ConfigFile::getmainContextServers() const {
    return (this->mainContextServers);
}

ConfigFile::~ConfigFile() {}


