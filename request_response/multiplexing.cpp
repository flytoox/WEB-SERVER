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

    FD_CLR(i, &readsd); FD_SET(i, &writesd);

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
            std::cerr << "Error: send(): " << strerror(errno) << std::endl;
        }
    }

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
                // std::cout << "YOU IN ???\n";

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
}

void funcMultiplexingBySelect(configFile &configurationServers) {

    std::vector<int> allSocketsVector;
    std::map<int, std::string> clientSocketsMap;
    std::map<int, std::string> clientsMap;
    fd_set readsd, writesd, allsd;
    std::map<int, Request> simultaneousRequests;
    int maxD = 0;

    getAllTheConfiguredSockets(configurationServers, allSocketsVector);
    FD_ZERO(&allsd);

    for (socket_iterator it = allSocketsVector.begin(); it != allSocketsVector.end(); ++it) {
        maxD = (*it);
        FD_SET((*it), &allsd);
    }

    struct timeval tv;

    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    for (FOREVER) {
        readsd = allsd;
        if (select(maxD + 1, &readsd, 0, 0, 0) < 0) {
            std::cerr << "Error: select(): " << strerror(errno) << std::endl;
        }

        for (int i = 0; i <= maxD; i++) {
            if ( ! FD_ISSET(i, &readsd)) {
                continue ;
            }

            socket_iterator readyToConnect = std::find(allSocketsVector.begin(), allSocketsVector.end(), i);
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
                maxD = std::max(connectSD, maxD);
                Request request;
                configureRequestClass(request, configurationServers, i);
                simultaneousRequests.insert(std::make_pair(connectSD, request));

            } else {
                try {
                    //* REQUEST
                    receiveRequestPerBuffer(simultaneousRequests, i, configurationServers, allsd);
                } catch (const char *error) {
                    //* RESPONSE
                    functionToSend(i, readsd, writesd, allsd, simultaneousRequests);
                }
            }
        }
    }
}
