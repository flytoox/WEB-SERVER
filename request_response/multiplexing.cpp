#include "../includes/webserve.hpp"

void getAllTheConfiguredSockets(configFile &configurationServers, std::vector<int> &allSocketsVector) {

    for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it)
        allSocketsVector.push_back(it->getSocketDescriptor());
}


static void functionToSend(int i , fd_set &readsd, fd_set &writesd, fd_set &allsd,std::map<int, Request>& simultaneousRequests) {

    FD_CLR(i, &readsd); 
    FD_SET(i, &writesd);

    // (void)readsd; (void)writesd;
    std::string res = simultaneousRequests[i].response.build();
    int sd;
    while (res.length()) {
        std::string chunk = "";
        if (res.length() > 65500) {
            chunk = res.substr(0, 65500); res.erase(0, 65500);
        } else {
            chunk = res; res.clear();
        }

        if ( FD_ISSET(i, &writesd) && (sd = send(i, chunk.c_str(), chunk.length(), 0)) == -1) {
            // std::cerr << "Error: send(): " << strerror(errno) << std::endl;
        }
    }
    // simultaneousRequests[i].setTimeout();

    //* Check of the connection is closed, if yes
    std::map<std::string, std::string> all = (simultaneousRequests[i]).getHttpRequestHeaders();
    if (all.find("Connection") != all.end()) {
        if ((all).find("Connection")->second == "keep-alive") {
            // std::cerr << i << std::endl;
            Request newRequest;
            std::cerr << all.find("Connection")->second << std::endl;
            newRequest.setDirectivesAndPages(simultaneousRequests[i].getDirectives(), simultaneousRequests[i].getPages());
            newRequest.setLocationsBlock(simultaneousRequests[i].getLocationsBlock());
            // newRequest.setTimeout();
            simultaneousRequests[i] = newRequest;
            return ;
        }
    }        
    close(i);
    FD_CLR(i, &allsd);
    FD_CLR(i, &writesd);
    simultaneousRequests.erase(i);
}

void configureRequestClass(Request &request, configFile &configurationServers, int i) {

    //Done: chooseTheServer that will be used
    Server serverUsed;

    for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {
        if (it->getSocketDescriptor() == i) {
            if ( it->duplicated == true ) {
                request.dup = true;
            }
            serverUsed = *it;
        }
    }

    std::map<std::string, std::string> serverDirectives = serverUsed.getdirectives();
    std::vector<std::map<std::string, std::string> > serverLocationsBlock = serverUsed.getlocationsBlock();
    request.RePort = serverUsed.prePort;
    request.ReHost = serverUsed.preHost;
    request.setDirectivesAndPages(serverDirectives, serverUsed.getPages());
    request.setLocationsBlock(serverLocationsBlock);
    // request.setTimeout();
}

void funcMultiplexingBySelect(configFile &configurationServers) {

    std::vector<int> ServersSD;
    fd_set readsd, writesd, allsd;
    std::map<int, Request> simultaneousRequests;
    std::set<int> Fds;

    getAllTheConfiguredSockets(configurationServers, ServersSD);
    FD_ZERO(&allsd);

    for (socket_iterator it = ServersSD.begin(); it != ServersSD.end(); it++) {
        Fds.insert(*it);
        FD_SET((*it), &allsd);
    }

    int responseD = 0;
    for (FOREVER) {    
        try {
            readsd = allsd;
            if (select(*Fds.rbegin() + 1, &readsd, 0, 0, 0) < 0) {
                std::cerr << "Error: select()" << std::endl;
                exit(1);
            }
            for (std::set<int>::iterator i = Fds.begin() ; i != Fds.end() && FD_ISSET(*i, &readsd); i++) {
                responseD = *i;
                if (std::find(ServersSD.begin(), ServersSD.end(), *i) == ServersSD.end()) {
                    time_t now = time(0);
                    time_t elapsedSeconds = now - simultaneousRequests[*i].getTimeout();
                    std::cout << *i << ' ' << ' '<< elapsedSeconds <<"\n";
                    if (elapsedSeconds >= 2) {

                        (simultaneousRequests[*i]).response = responseBuilder()
                            .addStatusLine("408")
                            .addContentType("text/html")
                            .addResponseBody(simultaneousRequests[*i].getPageStatus(408));
                        functionToSend(*i, readsd, writesd, allsd, simultaneousRequests);
                    }
                }
            }

            for (std::set<int>::iterator i = Fds.begin(); i != Fds.end(); i++) {
                responseD = *i;
                if (!FD_ISSET(*i, &readsd)) {
                    continue ;
                }

                socket_iterator readyToConnect = std::find(ServersSD.begin(), ServersSD.end(), *i);
                if (readyToConnect != ServersSD.end()) {
                    //! A Connection's been received
                    struct sockaddr_in clientAddress;
                    socklen_t addrlen =  sizeof(clientAddress);
                    int clientSD = accept(*readyToConnect , (struct sockaddr *)&clientAddress , &addrlen);
                    if (clientSD == -1 ) {
                        std::cerr << "Error: accept(): " << strerror(errno) << std::endl;
                        continue ;
                    }
                    FD_SET(clientSD, &allsd);
                    Fds.insert(clientSD);
                    Request request;
                    configureRequestClass(request, configurationServers, *i);
                    simultaneousRequests.insert(std::make_pair(clientSD, request));
                } else {
                    //* REQUEST
                    receiveRequestPerBuffer(simultaneousRequests, *i, configurationServers, allsd);
                }
            }
        } catch (const char *error) {
            //* RESPONSE
            functionToSend(responseD, readsd, writesd, allsd, simultaneousRequests);
        }
    }
}
