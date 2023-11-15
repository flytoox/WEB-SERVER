#include "configFile.hpp"
#include "Server.hpp"


configFile::configFile() {}


const std::vector<Server>& configFile::getServers() const {
    return servers;
}

void configFile::setServers(Server &initiateServer) {
    servers.push_back(initiateServer);
}


configFile::~configFile() {}

