#include "../includes/webserve.hpp"

void getAllTheConfiguredSockets(configFile &configurationServers, std::set<int> &allSocketsVector) {

    for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it)
        allSocketsVector.insert(it->getSocketDescriptor());
}


static void functionToSend(int i , fd_set &readsd, fd_set &writesd, fd_set &allsd,std::map<int, Request>& simultaneousRequests, std::set<int> &Fds) {

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
            std::cerr << "Error: send(): " << strerror(errno) << std::endl;
        }
    }
    // simultaneousRequests[i].setTimeout();

    //* Check of the connection is closed, if yes
    std::map<std::string, std::string> all = (simultaneousRequests[i]).getHttpRequestHeaders();
    if (all.find("Connection") != all.end() && (all).find("Connection")->second == "keep-alive") {
            // std::cerr << i << std::endl;
            Request newRequest;
            // std::cerr << all.find("Connection")->second << std::endl;
            newRequest.setDirectivesAndPages(simultaneousRequests[i].getDirectives(), simultaneousRequests[i].getPages());
            newRequest.setLocationsBlock(simultaneousRequests[i].getLocationsBlock());
            // newRequest.setTimeout();
            simultaneousRequests[i] = newRequest;
            std::cerr << "Keep-Alive" << std::endl;
            return ;
    }        
    std::cerr << "Close" << std::endl;
    close(i);
    FD_CLR(i, &allsd);
    FD_CLR(i, &writesd);
    simultaneousRequests.erase(i);
    Fds.erase(i);
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

void checkTimeOut(std::set<int> &Fds, std::set<int> &ServersSD, fd_set &allsd, fd_set &readsd, fd_set &writesd, std::map<int, Request> &simultaneousRequests, int &responseD){
    for (std::set<int>::iterator i = Fds.begin() ; i != Fds.end() && FD_ISSET(*i, &allsd); i++) {
        responseD = *i;
        if (std::find(ServersSD.begin(), ServersSD.end(), *i) == ServersSD.end()) {
            time_t now = time(0);
            time_t elapsedSeconds = now - simultaneousRequests[*i].getTimeout();
            if (elapsedSeconds >= 10) {

                (simultaneousRequests[*i]).response = responseBuilder()
                    .addStatusLine("408")
                    .addContentType("text/html")
                    .addResponseBody(simultaneousRequests[*i].getPageStatus(408));
                functionToSend(*i, readsd, writesd, allsd, simultaneousRequests, Fds);
            }
        }
    }
}

void funcMultiplexingBySelect(configFile &configurationServers) {

    std::set<int> ServersSD;
    fd_set readsd, writesd, allsd;
    std::map<int, Request> simultaneousRequests;
    std::set<int> Fds;

    getAllTheConfiguredSockets(configurationServers, ServersSD);
    FD_ZERO(&allsd);

    for (std::set<int>::iterator it = ServersSD.begin(); it != ServersSD.end(); it++) {
        Fds.insert(*it);
        FD_SET((*it), &allsd);
    }
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int responseD = 0;
    for (FOREVER) {    
        try {
            readsd = allsd;
            int ret = -1;
            while ((ret = select(*Fds.rbegin() + 1, &readsd, 0, 0, &timeout)) <= 0) {
                if (ret == -1) {
                    std::cerr << "Error: select()" << std::endl;
                    exit(1);
                }
                checkTimeOut(Fds, ServersSD, allsd, readsd, writesd, simultaneousRequests, responseD);
                readsd = allsd;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
            }
            checkTimeOut(Fds, ServersSD, allsd, readsd, writesd, simultaneousRequests, responseD);
            for (std::set<int>::iterator i = Fds.begin(); i != Fds.end(); i++) {
                if (!FD_ISSET(*i, &readsd)) {
                    continue ;
                }
                responseD = *i;
                if (find(ServersSD.begin(), ServersSD.end(), *i) != ServersSD.end()) {
                    //! A Connection's been received
                    struct sockaddr_in clientAddress;
                    socklen_t addrlen =  sizeof(clientAddress);
                    int clientSD = accept(*i , (struct sockaddr *)&clientAddress , &addrlen);
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
            std::cerr << error << std::endl;
            functionToSend(responseD, readsd, writesd, allsd, simultaneousRequests, Fds);
        }
    }
}
