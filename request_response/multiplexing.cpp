#include "../includes/webserve.hpp"

void getAllTheConfiguredSockets(configFile &configurationServers, std::vector<int> &allSocketsVector) {

    for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {
        //* GET SOCKET ON EACH SERVER
        Server serverIndex = (*it);
        int eachSocket = serverIndex.getSocketDescriptor();
        //* PUSH IT TO THE allSocketsVector
        allSocketsVector.push_back(eachSocket);
    }
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

    if ((simultaneousRequests[i]).getHttpRequestHeaders().find("Connection:") != (simultaneousRequests[i]).getHttpRequestHeaders().end()) {
        if (((simultaneousRequests[i]).getHttpRequestHeaders()).find("Connection:")->second == "closed") {
            close(i);
            FD_CLR(i, &allsd);
            FD_CLR(i, &writesd);
            simultaneousRequests.erase(i);
        } else {
            Request newRequest;

            newRequest.setDirectivesAndPages(simultaneousRequests[i].getDirectives(), simultaneousRequests[i].getPages());
            newRequest.setLocationsBlock(simultaneousRequests[i].getLocationsBlock());
            // newRequest.setTimeout();
            simultaneousRequests[i] = newRequest;
        }
    }
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

    std::vector<int> allSocketsVector;
    std::map<int, std::string> clientSocketsMap;
    std::map<int, std::string> clientsMap;
    fd_set readsd, writesd, allsd;
    std::map<int, Request> simultaneousRequests;
    std::set<int> Fds;

    getAllTheConfiguredSockets(configurationServers, allSocketsVector);
    FD_ZERO(&allsd);

    for (socket_iterator it = allSocketsVector.begin(); it != allSocketsVector.end(); ++it) {
        Fds.insert(*it);
        FD_SET((*it), &allsd);
    }

    int s;
    int responseD = 0;
    for (FOREVER) {    
        try {
            readsd = allsd;
            if ((s = select(*Fds.rbegin() + 1, &readsd, 0, 0, 0)) < 0) {
                std::cerr << "Error: select(): " << strerror(errno) << std::endl;
            }
            for (std::set<int>::iterator i = Fds.begin() ; i != Fds.end(); i++) {
                responseD = *i;
                if (std::find(allSocketsVector.begin(), allSocketsVector.end(), *i) == allSocketsVector.end()) {
                    time_t now = time(0);
                    time_t elapsedSeconds = now - simultaneousRequests[*i].getTimeout();
                    std::cerr << *i << ' ' << ' '<< elapsedSeconds <<"\n";
                    if (elapsedSeconds >= 2) {
                        std::cerr << elapsedSeconds << "\n";
                        (simultaneousRequests[*i]).response = responseBuilder()
                            .addStatusLine("408")
                            .addContentType("text/html")
                            .addResponseBody(simultaneousRequests[*i].getPageStatus(408));
                        functionToSend(responseD, readsd, writesd, allsd, simultaneousRequests);
                    }
                }
            }
            
            std::cout<< "3333333|S" << s << "|\n";

            for (std::set<int>::iterator i = Fds.begin(); i != Fds.end(); i++) {
                responseD = *i;
                if (!FD_ISSET(*i, &readsd)) {
                    continue ;
                }

                socket_iterator readyToConnect = std::find(allSocketsVector.begin(), allSocketsVector.end(), *i);
                if (readyToConnect != allSocketsVector.end()) {
                    //! A Connection's been received

                    struct sockaddr_in clientAddress;
                    socklen_t addrlen =  sizeof(clientAddress);
                    int connectSD = accept( (*readyToConnect) , (struct sockaddr *)&clientAddress , &addrlen);
                    if (connectSD == -1 ) {
                    std::cerr << "Error: accept(): " << strerror(errno) << std::endl;
                        continue ;
                    }
                    FD_SET(connectSD, &allsd);
                    Fds.insert(connectSD);
                    Request request;
                    configureRequestClass(request, configurationServers, *i);
                    simultaneousRequests.insert(std::make_pair(connectSD, request));

                } else
                        //* REQUEST
                        receiveRequestPerBuffer(simultaneousRequests, *i, configurationServers, allsd);
                }
            } catch (const char *error) {
                //* RESPONSE
                functionToSend(responseD, readsd, writesd, allsd, simultaneousRequests);
            }
    }
}
