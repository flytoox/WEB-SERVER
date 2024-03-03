#include "../includes/webserve.hpp"


configFile::configFile() {}


const std::vector<Server>& configFile::getServers() const {
    return servers;
}

void configFile::setServers(Server &initiateServer) {
    servers.push_back(initiateServer);
}

void configFile::setTheVector( std::vector<Server> servers) {
    this->servers = servers;
}
configFile::~configFile() {}



//TODO : take the directives and add them in each location block, if the directive exists in location block --> override
//TODO: Servers a Mr.Omar don't work 
//TODO: if all the servers don't have these [root, listen, host] -> generate an error > exit(1) 