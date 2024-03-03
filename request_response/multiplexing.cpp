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


    std::cout << "IT's ME WHO GOT HERE|................." << i << "|...\n";

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
            std::cout << "Error: send()" << std::endl;
        }
    }

    (void)max;
    //* Check of the connection is closed, if yes

    std::map<std::string, std::string> all = (simultaneousRequests[i]).getHttpRequestHeaders();

    // for (auto it : all) {
    //     std::cout << "---------------|" << it.first << "|\t|" << it.second << "|\n";
    // }

    if ((simultaneousRequests[i]).getHttpRequestHeaders().find("Connection:") != (simultaneousRequests[i]).getHttpRequestHeaders().end()) {
        // std::cout << "WHAAAAT CONNECTION|" << (simultaneousRequests[i]).getHttpRequestHeaders().find("Connection:")->second << "|\n";
        if (((simultaneousRequests[i]).getHttpRequestHeaders()).find("Connection:")->second == "closed") {
            close(i);
            FD_CLR(i, &allsd);

        }
    }            
    std::map<int, Request>::iterator it = simultaneousRequests.find(i);
    simultaneousRequests.erase(it);
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
    request.setDirectives(serverDirectives);
    request.setLocationsBlock(serverLocationsBlock);
}

void reCheckTheServer(configFile &configurationServers, std::string &header, Request &request) {

    try {
        Server serverReform;
        std::string v1 = header.substr(header.find("Host: ")); 
        std::string hostHeader = v1.substr(0, v1.find("\n"));
        std::string hostValue = hostHeader.substr(hostHeader.find(" ") + 1); 
        hostValue.erase(hostValue.length() - 1);

        // std::cout << "HOST HEADER|" << hostHeader << "|\n";
        std::cout << "HOST Value|" << hostValue << "|\n";

        if ( request.dup == true ) {

            for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {
                std::map<std::string, std::string>tmp = it->getdirectives();
                std::cout << "CAN YOU {PLEASE} |" << tmp["server_name"] << "|\n";
                std::cout << "LISTEN |" << tmp["listen"] << "| |" << request.RePort << "|\n";
                std::cout << "HOST |" << tmp["host"] << "| |" << request.ReHost << "|\n";
                if (tmp["server_name"] == hostValue && tmp["listen"] == request.RePort && tmp["host"] == request.ReHost ) {
                    serverReform = *it;
                    std::map<std::string, std::string> serverDirectives = serverReform.getdirectives();

                    for (auto it : serverDirectives) {
                        std::cout << "l3aar |" << it.first << "| |" << it.second << "|\n";
                    }
                    exit (0);
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
            .addResponseBody("<html><h1>400 Bad Request22</h1></html>");
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
            // exit (1);
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

            for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {
                    std::map<std::string, std::string> test = it->getdirectives();
                    std::cout << "------------------------------------\n";
                    for (auto ita : test) {
                        std::cout << "WHAT |" << ita.first << "| |" << ita.second << "|\n";
                    }
                    std::cout << "------------------------------------\n";
            }

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
                                std::cout << "DID YOU GET IN HERE.................|" << i << "|..\n";
                                reCheckTheServer(configurationServers, header, simultaneousRequests[i]);
                            }
                            parseAndSetRequestHeader(simultaneousRequests[i]);
                            (simultaneousRequests[i].setRequestBodyChunk(true));
                            // std::cout << "REACHED THIS|" << simultaneousRequests[i].reachedBodyLength << "|\n";
                            // std::cout << "CPNTENT-LENGTH:|" << (simultaneousRequests[i]).realContentLength << "|\n";
                            (simultaneousRequests[i]).reachedBodyLength = (simultaneousRequests[i].getRequestBody()).length();
                            if ((simultaneousRequests[i]).reachedBodyLength >= (simultaneousRequests[i]).realContentLength) {
                                std::cout << "REACHED THIS>>>>>>>>>>|" << simultaneousRequests[i].reachedBodyLength << "|\n";
                            // if ( recevRequestLen < 1024 ) {
                                // std::cout << "REAAALY}}}}}}}}}}}}}}}}}}}}\n";
                                parseRequestBody(simultaneousRequests[i]);
                                // getMethod(simultaneousRequests[i]);
                                checkRequestedHttpMethod(simultaneousRequests[i]);
                            }
                            }
                            
                    //  else if ( recevRequestLen < 1024  ) {

                    //         (simultaneousRequests[i]).response = responseBuilder()
                    //         .addStatusLine("400")
                    //         .addContentType("text/html")
                    //         .addResponseBody("<html><h1>400 Bad Request</h1></html>");

                    //         throw "400"; }

                         } catch (const char *err) {
                        functionToSend(maxD, i, readsd, writesd, allsd, simultaneousRequests);
                        std::cout << "Error From Request Header : " << err << std::endl;
                    }
                } else {
                    //! REQUEST BODY
                    // std::cout << "0- DID YOU EVEN GOT HREERE|||||||||||||||||||||||||||||||||||||||\n";
                    simultaneousRequests[i].setRequestBody(convert);

                    (simultaneousRequests[i]).reachedBodyLength = (simultaneousRequests[i].getRequestBody()).length();

                    // std::cout << "WHY |" << convert << "|\n";
                    // std::cout << "1- DID YOU EVEN GOT HREERE|||||||||||||||||||||||||||||||||||||||\n";

                    //TODO: here insert the max here check length
                    //TODO: 
                    try {
                        // if (recevRequestLen < 1024) {
                            // std::cout << "REACHED THIS|" << simultaneousRequests[i].reachedBodyLength << "|\n";
                            // std::cout << "CPNTENT-LENGTH:|" << (simultaneousRequests[i]).realContentLength << "|\n";

                        if (((simultaneousRequests[i]).reachedBodyLength >= (simultaneousRequests[i]).realContentLength) || \
                            ((((simultaneousRequests[i]).getHttpRequestHeaders()).find("Transfer-Encoding:") != (simultaneousRequests[i]).getHttpRequestHeaders().end()) && \
                                (simultaneousRequests[i].getRequestBody().find("0\r\n\r\n") != std::string::npos)) ) {
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
