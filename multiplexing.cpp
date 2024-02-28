#include "webserve.hpp"
#include <sys/errno.h>

// bool reCheck = false;

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

        // std::string res = "";
    (void)readsd; (void)writesd;
        // std::cout << "*********KOKOKOKOKOOKOKOK***********\n";

        // FD_CLR(i, &readsd); 
        // FD_SET(i, &writesd);

        // std::cout << "\nRESPONSE\n";

        // std::vector<std::string> chunkedResponse = simultaneousRequests[i].getChunkedResponse();
        // std::string textResponse = simultaneousRequests[i].getTextResponse();
        // std::vector<std::string> multipartReponse = simultaneousRequests[i].getMultipartReponse();
        // std::map<std::string, std::string> urlencodedResponse = simultaneousRequests[i].getUrlencodedResponse();


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

        // for (auto it : simultaneousRequests[i].getResponseVector()) {
        //     res += it;
        // }


        std::string res = simultaneousRequests[i].response.build();

        // std::cerr << "REPOMMSEEEEE|" << res << "|\n";

        // std::cout << "\n-----------------------------------------------\n";
        // std::cout << "|" << res << "|\n";
        // std::cout << "\n-----------------------------------------------\n";

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
            // FD_ISSET(i, &writesd) &&
            if ( (sd = send(i, chunk.c_str(), chunk.length(), 0)) == -1) {
                std::cout << "Error: send()" << std::endl; exit (1);
            }


            //std::cout << "HEADER:|" << chunk.c_str() << "|\n";
            //std::cout << " res : " << res.length() << "|\t sd : |" << sd << "|\n";
        } 

        // std::cout << "---> |" << i << "|\n";


        // (void)allsd;
        //(void)max;
        close(i);
        FD_CLR(i, &allsd);
        if (i == max)
            max--;
    
        std::map<int, Request>::iterator it = simultaneousRequests.find(i); 
        simultaneousRequests.erase(it);

        // for (auto it: simultaneousRequests)
        //     std::cout << "fds = " << it.first << std::endl;
        
        // std::cout << "****POPOPOPOPOPoPoPOP**********\n";
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
                // std::cout << "\n<--- Inside -->\n";
                // std::cout << "server_name |" << hostValue << "|\n";
                // std::cout << "Port |" << tmp["port"] << "|\n";
                // std::cout << "Host |" << tmp["host"] << "|\n";
                if (tmp["server_name"] == hostValue && tmp["listen"] == request.RePort && tmp["host"] == request.ReHost ) {
                    // std::cout << "Port |" << request.RePort << "|\n";
                    // std::cout << "Host |" << request.ReHost << "|\n";
                    // std::cout << "Server|" << tmp["server_name"] << "|\n";
                    serverReform = *it;
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
bool chk = true;

void functionTest(Request& r) {
    (void)r;
    // if (chk)
    //     std::cerr << r.getRequestHeader() << std::endl;
    // std::cerr << "done" << std::endl;
    return ;
}

void push_convert(std::string &convert, char *buffer, int r) {
    for (int i = 0; i != r; i++)
        convert.push_back(buffer[i]);
}
void dd() {}
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
    std::cout << "VALUE |" << max << "|\n";
    int a;

    for (FOREVER) {
        readsd = allsd;
        char buffer[1024] = {0};
        //std::cout << "  max = " << max << std::endl;
        // if (max == 5) {
        //     std::cout << "      here adding 5" << std::endl;
        //     FD_SET(5, &readsd);
        // }
        dd();
        if ((a = select(max + 1, &readsd, 0, 0, 0)) < 0) {
            std::cerr << "Error: select() fail" << std::endl;
            exit (1);
        }
        // std::cout << "      select socket ready = " << a << std::endl;
        // std::cout << "  going to dd()"<< std::endl;
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
                //std::cout << "  here a new connection = " << connectSD << std::endl;

                FD_SET(connectSD, &allsd);
                if (connectSD > max) {
                    max = connectSD ;
                }

                Request request;


                configureRequestClass(request, configurationServers, i);
                simultaneousRequests.insert(std::make_pair(connectSD, request));
                //break ;
                //simultaneousRequests[connectSD] = request;

            } else {
            
                std::string res; 
                int recevRequestLen = recv(i , buffer, 1024, 0);
                // int recevRequestLen = read(i, buffer, 1024);
                //std::cout << "ok ***** = " << recevRequestLen << << std::endl;
                functionTest(simultaneousRequests[i]);
                
                if (recevRequestLen < 0) {
                    std::cout << "Error: recev()" << std::endl;
                    close(i), FD_CLR(i, &allsd); continue ;
                }

                // std::string convert(buffer, recevRequestLen);
                std::string convert;
                push_convert(convert, buffer, recevRequestLen);
                // std::cout << "L3ar|" << convert << "|\n";
                // for (int i = 0; i != recevRequestLen; i++)
                //     convert.push_back(buffer[i]);
                // checkRightServer();
                //* REQUEST 
                if ( ! (simultaneousRequests[i].getRequestBodyChunk()) ) {
                    //! REQUEST HEADER
                    simultaneousRequests[i].setRequestHeader(convert);
                    try {
                    if ( ((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") != std::string::npos ) {
                            std::string header = (simultaneousRequests[i]).getRequestHeader();
                            // std::cout << "PRI|"  << ((simultaneousRequests[i]).getRequestHeader()) << std::endl;
                            //DONE1: this unction must check the server only once! 
                            if ((simultaneousRequests[i]).reCheck != true) {
                                //* Fix this 
                                (simultaneousRequests[i]).reCheck = true;
                                reCheckTheServer(configurationServers, header, simultaneousRequests[i]);
                            }
                            parseAndSetRequestHeader(simultaneousRequests[i]);
                            // mapConstIterator mapIt = ((simultaneousRequests[i]).getHttpRequestHeaders()).find("Transfer-Encoding:");
                            // std::string transferEncoding = mapIt->second; (&& ! ( transferEncoding.empty() ) && transferEncoding != "chunked")
                            if ( recevRequestLen < 1024 ) {
                                // std::cout << "DID YOU GET IN HERE ?|" << simultaneousRequests[i].getHttpVerb() << "|\t| i: " << i << "|\n";
                                // std::cout << "DIDN'T WORK|" << convert << "|" << std::endl;
                                parseRequestBody(simultaneousRequests[i]);
                                checkRequestedHttpMethod(simultaneousRequests[i]);
                            }
                            
                    } else if ( recevRequestLen < 1024  ) {
                        //if ( ((simultaneousRequests[i]).getRequestHeader()).empty() ) {

                            // std::cout << "NOOOOOOOOO|\n";

                            (simultaneousRequests[i]).response = responseBuilder()
                            .addStatusLine("400")
                            .addContentType("text/html")
                            .addResponseBody("<html><h1>400 Bad Request</h1></html>");

                            throw "400";

                        //}
                    } } catch (const char *err) {
                        functionToSend(max, i, readsd, writesd, allsd, simultaneousRequests);
                        std::cout << "Error From Request Header : " << err << std::endl;
                        // functionToSend(i, readsd, writesd, simultaneousRequests);
                    }
                } else {
                    //! REQUEST BODY
                    simultaneousRequests[i].setRequestBody(convert);


                    // std::cout << "L3ar|" << simultaneousRequests[i].getRequestBody() << "|\n";

                    //TODO: here insert the max here check length

                    // mapConstIterator mapIt = ((simultaneousRequests[i]).getHttpRequestHeaders()).find("Transfer-Encoding:");
                    // std::string transferEncoding = mapIt->second; (&& ! ( transferEncoding.empty() ) && transferEncoding != "chunked")
                    try {
                        if (recevRequestLen < 1024) {
                            parseRequestBody((simultaneousRequests[i]));
                            // std::cerr << simultaneousRequests[i].getRequestBody();   
                            checkRequestedHttpMethod(simultaneousRequests[i]);
                        }
                    } catch (const char *err) {
                        functionToSend(max, i, readsd, writesd, allsd, simultaneousRequests);
                        std::cout << "Error From Request Body : " << err << std::endl; 
                    }
                }
            }
        }
    }
}
