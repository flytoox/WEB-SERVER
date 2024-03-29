#include "../includes/webserve.hpp"

void getAllTheConfiguredSockets(configFile &configurationServers, std::set<int> &allSocketsVector) {

    for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it)
        allSocketsVector.insert(it->getSocketDescriptor());
}


static void functionToSend(int i , fd_set &readsd, fd_set &allsd,std::map<int, Request>& requests, std::set<int> &Fds, bool isGetFile) {

    FD_CLR(i, &readsd);
    if (!isGetFile) {
        std::string &res = requests[i].response.build();
        size_t pos = 0;
        while (pos < res.length()) {
            std::string chunk = "";
            chunk = res.substr(pos, 65500);
            pos += 65500;
            if (send(i, chunk.c_str(), chunk.length(), 0) == -1) {
                std::cerr << "Error: send()" << std::endl;
            }
        }
    }

    //* Check of the connection is closed, if yes
    std::map<std::string, std::string> all = (requests[i]).getHttpRequestHeaders();
    if (all.find("Connection") != all.end() && (all).find("Connection")->second == "keep-alive") {
        Request newRequest;
        newRequest.setDirectivesAndPages(requests[i].getDirectives(), requests[i].getPages());
        newRequest.setLocationsBlock(requests[i].getLocationsBlock());
        newRequest.FD = i;
        requests[i] = newRequest;
        return ;
    }
    close(i);
    FD_CLR(i, &allsd);
    requests.erase(i);
    Fds.erase(i);
}

void configureRequestClass(Request &request, configFile &configurationServers, int i) {

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
}

void checkTimeOut(std::set<int> &Fds, std::set<int> &ServersSD, fd_set &allsd, fd_set &readsd, std::map<int, Request> &requests, int &responseD){
    for (std::set<int>::iterator i = Fds.begin() ; i != Fds.end() && FD_ISSET(*i, &allsd); i++) {
        std::cout << *i << std::endl;
        if (!requests[*i].checkTimeout) continue;
        responseD = *i;
        if (std::find(ServersSD.begin(), ServersSD.end(), *i) == ServersSD.end()) {
            time_t now = time(0);
            time_t elapsedSeconds = now - requests[*i].getTimeout();
            std::cerr << "Elapsed seconds: " << elapsedSeconds << std::endl;
            if (elapsedSeconds >= 10) {
                (requests[*i]).response = responseBuilder()
                    .addStatusLine("408")
                    .addContentType("text/html")
                    .addResponseBody(requests[*i].getPageStatus(408));
                functionToSend(*i, readsd, allsd, requests, Fds, false);
            }
        }
    }
}

void funcMultiplexingBySelect(configFile &configurationServers) {

    std::set<int> ServersSD;
    fd_set readsd, allsd, writesd;
    std::map<int, Request> requests;
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
            writesd = allsd;
            int ret = -1;
            while ((ret = select(*Fds.rbegin() + 1, &readsd, &writesd, 0, &timeout)) <= 0) {
                if (ret == -1) {
                    std::cerr << "Error: select()" << std::endl;
                    exit(1);
                }
                checkTimeOut(Fds, ServersSD, allsd, readsd, requests, responseD);
                readsd = allsd;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
            }
            checkTimeOut(Fds, ServersSD, allsd, readsd, requests, responseD);
            for (std::set<int>::iterator i = Fds.begin(); i != Fds.end(); i++) {
                if (!FD_ISSET(*i, &readsd) && !FD_ISSET(*i, &writesd)) {
                    continue ;
                }
                responseD = *i;
                if (find(ServersSD.begin(), ServersSD.end(), *i) != ServersSD.end()) {
                    //! A Connection's been received
                    struct sockaddr_in clientAddress;
                    socklen_t addrlen =  sizeof(clientAddress);
                    int clientSD = accept(*i , (struct sockaddr *)&clientAddress , &addrlen);
                    if (clientSD == -1 ) {
                        std::cerr << "Error: accept()" << std::endl;
                        continue ;
                    }
                    FD_SET(clientSD, &allsd);
                    Fds.insert(clientSD);
                    Request request;
                    configureRequestClass(request, configurationServers, *i);
                    request.FD = clientSD;
                    requests.insert(std::make_pair(clientSD, request));
                } else {
                    //* REQUEST
                    receiveRequestPerBuffer(requests[*i], *i, configurationServers, allsd, readsd);
                }
            }
        } catch (const char *error) {
            //* RESPONSE
            std::cerr << "Error:: " << error << std::endl;
            std::string err = error;
            functionToSend(responseD, readsd, allsd, requests, Fds, err == "GET_FILE");
        }
    }
}
