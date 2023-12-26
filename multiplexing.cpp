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

        // std::cout << "\nRESPONSE\n";

        std::vector<std::string> chunkedResponse = simultaneousRequests[i].getChunkedResponse();
        std::string textResponse = simultaneousRequests[i].getTextResponse();
        std::vector<std::string> multipartReponse = simultaneousRequests[i].getMultipartReponse();
        std::map<std::string, std::string> urlencodedResponse = simultaneousRequests[i].getUrlencodedResponse();


        // std::cerr << textResponse << std::endl;
        // std::cout << "start" << std::endl;
        // std::cout << multipartReponse[0].size() << std::endl;
        // for (auto it : multipartReponse) {+
        //     std::cerr << it << std::endl;
        // }
        // std::cout << "finished" << std::endl;

        // res +=  "HTTP/1.1 200 OK\r\n";
        // res += "Content-Length: "; res += (simultaneousRequests[i].getrequestOutputTest()).length() ; res += "\r\n";
        // res += "Content-Type: text/html; charset=UTF-8\r\n";
        // res += "\r\n";
        // res += (simultaneousRequests[i].getrequestOutputTest());

        for (auto it : simultaneousRequests[i].getResponseVector()) {
            res += it;
        }


        // chunk = res.length() < 6000 ? res : res.substr(0, 6000) ;
        // res.length() < 6000 ? res.erase() : res.erase(0, 6000) ;
        // if (FD_ISSET(i, &writesd) && send(i, res.c_str(), res.length(), 0) == -1) {
        //     std::cout << "done herte" << std::endl;
        //     std::cout << "Error: send()" << std::endl; exit (1);
        // }
        int sd;
        while ( res.length() ) {

            std::string chunk = "";
            if (res.length() > 65500) {
                chunk = res.substr(0, 65500); res.erase(0, 65500);
            } else {
                chunk = res; res.clear();
            }

            if (FD_ISSET(i, &writesd) && (sd = send(i, chunk.c_str(), chunk.length(), 0)) == -1) {
                std::cout << "Error: send()" << std::endl; exit (1);
            }


            //std::cout << "HEADER:|" << chunk.c_str() << "|\n";
            //std::cout << " res : " << res.length() << "|\t sd : |" << sd << "|\n";
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
    std::cout << "Port |" << request.RePort << "|\n";
    std::cout << "Host |" << request.ReHost << "|\n";
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
                std::cout << "\n<--- Inside -->\n";
                std::cout << "server_name |" << hostValue << "|\n";
                std::cout << "Port |" << tmp["port"] << "|\n";
                std::cout << "Host |" << tmp["host"] << "|\n";
                if (tmp["server_name"] == hostValue && tmp["listen"] == request.RePort && tmp["host"] == request.ReHost ) {
                    // std::cout << "Port |" << request.RePort << "|\n";
                    // std::cout << "Host |" << request.ReHost << "|\n";
                    // std::cout << "Server|" << tmp["server_name"] << "|\n";
                    serverReform = *it;
                    std::cout << "passed" ;
                    std::map<std::string, std::string> serverDirectives = serverReform.getdirectives();
                    std::vector<std::map<std::string, std::string> > serverLocationsBlock = serverReform.getlocationsBlock();

                    request.setDirectives(serverDirectives);
                    request.setLocationsBlock(serverLocationsBlock);
                    break ;
                }
            }


    } } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }


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
                // checkRightServer();
                //* REQUEST 
                if ( ! (simultaneousRequests[i].getRequestBodyChunk()) ) {
                    //! REQUEST HEADER
                    simultaneousRequests[i].setRequestHeader(convert);
                    try {
                    if ( ((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") != std::string::npos ) {
                            std::string header = (simultaneousRequests[i]).getRequestHeader();
                            //TODO: this unction must check the server only once! 
                            reCheckTheServer(configurationServers, header, simultaneousRequests[i]);
                            parseAndSetRequestHeader(simultaneousRequests[i]);
                            // mapConstIterator mapIt = ((simultaneousRequests[i]).getHttpRequestHeaders()).find("Transfer-Encoding:");
                            // std::string transferEncoding = mapIt->second; (&& ! ( transferEncoding.empty() ) && transferEncoding != "chunked")
                            if ( recevRequestLen < 1024 ) {
                                // std::cout << (simultaneousRequests[i]).getRequestBody() << std::endl;
                                parseRequestBody(simultaneousRequests[i]);
                                checkRequestedHttpMethod(simultaneousRequests[i]);
                            }
                            
                    } else if ( recevRequestLen ) {
                        if ( ((simultaneousRequests[i]).getRequestHeader()).empty() ) {

                            std::string response = "HTTP/1.1 204 No Content\r\n"; (simultaneousRequests[i]).setResponseVector(response);
                            response = "Content-Type: text/html\r\n"; (simultaneousRequests[i]).setResponseVector(response);
                            response = "Content-Length: 36\r\n\r\n"; (simultaneousRequests[i]).setResponseVector(response);
                            response = "<html><h1>204 No Content</h1></html>\r\n"; (simultaneousRequests[i]).setResponseVector(response);
                            throw "204" ; 

                        } else {
                            parseRequestBody(simultaneousRequests[i]); checkRequestedHttpMethod(simultaneousRequests[i]);
                        }
                    } } catch (const char *err) {
                        functionToSend(i, readsd, writesd, allsd, simultaneousRequests);
                        std::cout << "Error From Request Header : " << err << std::endl;
                        // functionToSend(i, readsd, writesd, simultaneousRequests);
                    }
                } else {
                    //! REQUEST BODY
                    simultaneousRequests[i].setRequestBody(convert);

                    //? here insert the max here

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
