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


static void functionToSend(int &max, int i , fd_set &readsd, fd_set &writesd, fd_set &allsd,std::map<int, Request>& simultaneousRequests) {

    (void)readsd; (void)writesd;
    std::string res = simultaneousRequests[i].response.build();
    int sd;
    while (res.length()) {
        std::string chunk = "";
        if (res.length() > 65500) {
            chunk = res.substr(0, 65500); res.erase(0, 65500);
        } else {
            chunk = res; res.clear();
        }
        if ((sd = send(i, chunk.c_str(), chunk.length(), 0)) == -1) {
            std::cout << "Error: send()" << std::endl; exit (1);
        }
    }

    close(i);
    FD_CLR(i, &allsd);
    if (i == max)
        max--;
    std::map<int, Request>::iterator it = simultaneousRequests.find(i); 
    simultaneousRequests.erase(it);
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
    request.setDirectives(serverDirectives);
    request.setLocationsBlock(serverLocationsBlock);
}

void reCheckTheServer(configFile &configurationServers, std::string &header, Request &request) {

    try {
        Server serverReform;
        std::string v1 = header.substr(header.find("Host: ")); std::string hostHeader = v1.substr(0, v1.find("\n"));
        std::string hostValue = hostHeader.substr(hostHeader.find(" ") + 1); hostValue.erase(hostValue.length() - 1);
        if ( request.dup == true ) {

            for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {
                std::map<std::string, std::string>tmp = it->getdirectives();
                if (tmp["server_name"] == hostValue && tmp["listen"] == request.RePort && tmp["host"] == request.ReHost ) {
                    serverReform = *it;
                    std::map<std::string, std::string> serverDirectives = serverReform.getdirectives();
                    std::vector<std::map<std::string, std::string> > serverLocationsBlock = serverReform.getlocationsBlock();
                    request.setDirectives(serverDirectives);
                    request.setLocationsBlock(serverLocationsBlock);
                    break ;
                }
            }
    } } catch (std::exception &e) {
        request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400";
        std::cout << e.what() << std::endl;
    }

}

void push_convert(std::string &convert, char *buffer, int r) {
    for (int i = 0; i != r; i++)
        convert.push_back(buffer[i]);
}

void funcMultiplexingBySelect(configFile &configurationServers) {

    std::vector<int> allSocketsVector;

    std::map<int, std::string> clientSocketsMap;
    std::map<int, std::string> clientsMap;
    fd_set readsd, writesd, allsd;
    (void)writesd;


    std::map<int, Request> simultaneousRequests;

    getAllTheConfiguredSockets(configurationServers, allSocketsVector);

    FD_ZERO(&allsd);
    int maxD = 0;

    for (socket_iterator it = allSocketsVector.begin(); it != allSocketsVector.end(); ++it) {
        maxD = (*it);
        FD_SET((*it), &allsd);
    }    


    for (FOREVER) {
        readsd = allsd;
        char buffer[1024] = {0};
        if (select(maxD + 1, &readsd, 0, 0, 0) < 0) {
            std::cerr << "Error: select() fail" << std::endl;
            exit (1);
        }
        for (int i = 0; i <= maxD; i++) {

            if ( ! FD_ISSET(i, &readsd)) {
                continue ;
            }
            

            socket_iterator readyToConnect = std::find(allSocketsVector.begin(), allSocketsVector.end(), i);
            if (readyToConnect != allSocketsVector.end()) {
                struct sockaddr_in clientAddress;
                socklen_t addrlen =  sizeof(clientAddress);

                int connectSD = accept( (*readyToConnect) , (struct sockaddr *)&clientAddress , &addrlen);
                if (connectSD == -1 ) {
                    continue ;
                };

                FD_SET(connectSD, &allsd);
                maxD = std::max(connectSD, maxD);

                Request request;

                configureRequestClass(request, configurationServers, i);
                simultaneousRequests.insert(std::make_pair(connectSD, request));

            } else {
            
                std::string res; 
                int recevRequestLen = recv(i , buffer, 1024, 0);
                
                if (recevRequestLen < 0) {
                    std::cout << "Error: recev()" << std::endl;
                    close(i), FD_CLR(i, &allsd); continue ;
                }

                std::string convert;
                push_convert(convert, buffer, recevRequestLen);

                //* REQUEST 
                if ( ! (simultaneousRequests[i].getRequestBodyChunk()) ) {
                    //! REQUEST HEADER
                    simultaneousRequests[i].setRequestHeader(convert);
                    try {
                    if ( ((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") != std::string::npos ) {
                            std::string header = (simultaneousRequests[i]).getRequestHeader();
                            //DONE1: this unction must check the server only once! 
                            if ((simultaneousRequests[i]).reCheck != true) {
                                //* Fix this 
                                (simultaneousRequests[i]).reCheck = true;
                                reCheckTheServer(configurationServers, header, simultaneousRequests[i]);
                            }
                            parseAndSetRequestHeader(simultaneousRequests[i]);
                            if ( recevRequestLen < 1024 ) {
                                parseRequestBody(simultaneousRequests[i]);
                                checkRequestedHttpMethod(simultaneousRequests[i]);
                            }
                            
                    } else if ( recevRequestLen < 1024  ) {

                            (simultaneousRequests[i]).response = responseBuilder()
                            .addStatusLine("400")
                            .addContentType("text/html")
                            .addResponseBody("<html><h1>400 Bad Request</h1></html>");

                            throw "400";

                    } } catch (const char *err) {
                        functionToSend(maxD, i, readsd, writesd, allsd, simultaneousRequests);
                        std::cout << "Error From Request Header : " << err << std::endl;
                    }
                } else {
                    //! REQUEST BODY
                    simultaneousRequests[i].setRequestBody(convert);

                    //TODO: here insert the max here check length
                    try {
                        if (recevRequestLen < 1024) {
                            parseRequestBody((simultaneousRequests[i]));
                            checkRequestedHttpMethod(simultaneousRequests[i]);
                        }
                    } catch (const char *err) {
                        functionToSend(maxD, i, readsd, writesd, allsd, simultaneousRequests);
                        std::cerr << "Error From Request Body : " << err << std::endl; 
                    }
                }
            }
        }
    }
}
