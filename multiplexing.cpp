#include "webserve.hpp"
#include <sys/errno.h>

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

        std::string res = "";
    

        FD_CLR(i, &readsd); FD_SET(i, &writesd);

        // res +=  "HTTP/1.1 200 OK\r\n";
        // res += "Content-Length: "; res += (simultaneousRequests[i].getrequestOutputTest()).length() ; res += "\r\n";
        // res += "Content-Type: text/html; charset=UTF-8\r\n";
        // res += "\r\n";
        // res += (simultaneousRequests[i].getrequestOutputTest());

        for (auto it : simultaneousRequests[i].getResponseVector()) {
            res += it;
        }

        if (FD_ISSET(i, &writesd) && send(i, res.c_str(), res.length(), 0) < 0) {
            std::cout << "Error: send()" << std::endl; exit (1);
        }
        close(i) ; FD_CLR(i, &allsd);
    
        std::map<int, Request>::iterator it = simultaneousRequests.find(i); 
        simultaneousRequests.erase(it);
}

void configureRequestClass(Request &request, configFile &configurationServers, int i) {

    //Done: chooseTheServer that will be used 
    Server serverUsed;

    for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {

        if (it->getSocketDescriptor() == i) {
            serverUsed = *it;
        }
    }

    std::map<std::string, std::string> serverDirectives = serverUsed.getdirectives();
    std::vector<std::map<std::string, std::string> > serverLocationsBlock = serverUsed.getlocationsBlock();

    request.setDirectives(serverDirectives);
    request.setLocationsBlock(serverLocationsBlock);
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
    int max = 0;

    for (socket_iterator it = allSocketsVector.begin(); it != allSocketsVector.end(); ++it) {
        max = (*it);
        FD_SET((*it), &allsd);
    }    

    for (FOREVER) {

        readsd = allsd;
        char buffer[1024] = {0};

        if (select(max + 1, &readsd, 0, 0, 0) < 0) {
            std::cerr << "Error: select() fail" << std::endl;
            exit (1);
        }

        for (int i = 0; i <= max; i++) {

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
                }

                FD_SET(connectSD, &allsd);
                if (connectSD > max) {
                    max = connectSD ;
                }

                Request request;


                configureRequestClass(request, configurationServers, i);
                simultaneousRequests.insert(std::make_pair(connectSD, request));
                //simultaneousRequests[connectSD] = request;

            } else {
            
                std::string res; 
                int recevRequestLen = recv(i , buffer, sizeof(buffer), 0);

                if (recevRequestLen < 0) {
                    std::cout << "Error: recev()" << std::endl;
                    close(i), FD_CLR(i, &allsd); continue ;
                }

                std::string convert(buffer, recevRequestLen);

                //* REQUEST 
                if ( ! (simultaneousRequests[i].getRequestBodyChunk()) ) {
                    //! REQUEST HEADER
                    simultaneousRequests[i].setRequestHeader(convert);
                    if ( ((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") != std::string::npos ) {
                        try {
                            parseAndSetRequestHeader(simultaneousRequests[i]);
                            // mapConstIterator mapIt = ((simultaneousRequests[i]).getHttpRequestHeaders()).find("Transfer-Encoding:");
                            // std::string transferEncoding = mapIt->second; (&& ! ( transferEncoding.empty() ) && transferEncoding != "chunked")
                            if (recevRequestLen < 1024 ) {
                                // std::cout << (simultaneousRequests[i]).getRequestBody() << std::endl;
                                parseRequestBody(simultaneousRequests[i]);
                                checkRequestedHttpMethod(simultaneousRequests[i]);
                            }
                            
                        } catch (const char *err) {
                            functionToSend(i, readsd, writesd, allsd, simultaneousRequests);
                            std::cout << "Error From Request Header : " << err << std::endl;
                            // functionToSend(i, readsd, writesd, simultaneousRequests);
                        }
                    } 
                } else {
                    //! REQUEST BODY
                    simultaneousRequests[i].setRequestBody(convert);
                    // mapConstIterator mapIt = ((simultaneousRequests[i]).getHttpRequestHeaders()).find("Transfer-Encoding:");
                    // std::string transferEncoding = mapIt->second; (&& ! ( transferEncoding.empty() ) && transferEncoding != "chunked")
                    try {
                        if (recevRequestLen < 1024) {
                            //std::cout << (simultaneousRequests[i]).getRequestBody() << std::endl;
                            parseRequestBody((simultaneousRequests[i]));
                            checkRequestedHttpMethod(simultaneousRequests[i]);
                        }
                    } catch (const char *err) {
                        functionToSend(i, readsd, writesd, allsd, simultaneousRequests);
                        std::cout << "Error From Request Body : " << err << std::endl; 
                    }
                }
            }
        }
    }
}
