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


void configureRequestClass(Request &request, configFile &configurationServers, int i) {

    //Done: chooseTheServer that will be used 
    i -= 3;
    int counter = -1;
    Server serverUsed;

    for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {

        if (++counter == i) {
            serverUsed = (*it);
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

        for (int i = 0; i <= max; i++ ) {

            if ( ! FD_ISSET(i ,&readsd)) {
                continue ;
            }

            socket_iterator readyToConnect = std::find(allSocketsVector.begin(), allSocketsVector.end(), i);
            //std::string joiningBufferHeader = ""; std::string joiningBufferBody = "";
            bool response = false;
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
                //simultaneousRequests.insert(std::make_pair(i, request));
                simultaneousRequests[i] = request;

            } else {
                std::string res;


                    int recevRequestLen = recv(i , buffer, sizeof(buffer), 0);

                    if (recevRequestLen < 0) {
                        std::cout << "Error() : recv()" << std::endl;
                        close(i); FD_CLR(i, &allsd); continue ;
                    }

                    std::string convert(buffer, recevRequestLen);
                    if (! response ) 
                    {
                        //std::cout << "buffer: " << convert << std::endl;
                        (simultaneousRequests[i]).setRequestHeader(convert);
                        //std::cout << "ptr: " << convert.find("GET") << std::endl;
                        //write(0, &joiningBufferHeader, sizeof(joiningBufferHeader));
                        if ( ((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") != std::string::npos ) {
            
                            //(simultaneousRequests[i]).setRequestHeader(joiningBufferHeader);
                            parseAndSetRequestHeader((simultaneousRequests[i]));
                            response = true;
                            //executeAndSend = true;
                        }
                        response = true;

                    } 
                    if ( response )
                    {
                        
                        // executeAndSend = true;
                        // joiningBufferBody += convert;
                        // if (recevRequestLen == 0 || convert.empty() || convert.size() < 2000) {
                        //     (simultaneousRequests[index]).setRequestBody(joiningBufferBody);
                        //     parseRequestBody(request);
                        //     executeAndSend = true;
                        // }
                        res +=  "HTTP/1.1 200 OK\r\n";
                        res += "Content-Length: ";
                        res += (simultaneousRequests[i].getrequestOutputTest()).length();
                        res += "\r\n";
                        res += "Content-Type: text/html\r\n";
                        res += (simultaneousRequests[i].getrequestOutputTest());
                        res += "\r\n";
                    }
                    //FD_CLR(i, &readsd); FD_SET(i, &writesd);
                    response = false;
                // std::cout << "HEREEE<-->HEREEEEE" << std::endl;
                std::cout << res << "\n";
                //if (FD_ISSET(i, &writesd))
                send(i, res.c_str(), res.length(), 0);
                // if (send(i, res.c_str(), sizeof(res), 0) < 0) {
                //     std::cerr << "Error() : send() " << std::endl;
                // }
                //*changed
                close(i); FD_CLR(i, &allsd);
            }
            //close after finishing (getting the request and sending the response)
        }
    }
}