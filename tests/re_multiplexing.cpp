#include "webserve.hpp"

//*SIEGE command: siege --file=file.txt -v --reps=1000 > test

void getAllTheConfiguredSockets(configFile &configurationServers, std::vector<int> &allSocketsVector) {

    for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {
        //* GET SOCKET ON EACH SERVER
        Server serverIndex = (*it);
        int eachSocket = serverIndex.getSocketDescriptor();
    
        //* PUSH IT TO THE allSocketsVector
        allSocketsVector.push_back(eachSocket);
    }
}

void funcMultiplexingBySelect(configFile &configurationServers) {

    std::vector<int> allSocketsVector;


    //* For saving the connectfd (RESPONSE)
    std::map<int, std::string> clientSocketsMap;
    std::map<int, std::string> clientsMap;
    fd_set readsd, writesd, allsd;

    getAllTheConfiguredSockets(configurationServers, allSocketsVector);

    FD_ZERO(&allsd);
    int max = 0;

    for (socket_iterator it = allSocketsVector.begin(); it != allSocketsVector.end(); ++it) {
        max = (*it);
        FD_SET((*it), &allsd);
    }

    //int sokt = (*allSocketsVector.begin()); FD_SET(sokt, &allsd); int max = sokt;

    std::string requestOutput;

    for (FOREVER)
    {
        //! readset is the one returned from select();
        //! allsd is the one used for storing the socket descriptors
        //readsd = writesd = allsd;
        readsd = allsd;
        char buffer[2000] = {0};
        if (select(max + 1, &readsd, &writesd, NULL, 0) < 0) {
            std::cerr << "Error: select() fail" << std::endl;
        }

        for (int i = 0; i <= max; i++) {
            // run through all the sockets
            if ( ! FD_ISSET(i, &readsd) )  
                continue ;

            //* loop through the servers socket descriptors
            socket_iterator readyToConnect = std::find(allSocketsVector.begin(), allSocketsVector.end(), i);

            if ( readyToConnect != allSocketsVector.end() ) {
                //* check if that socket is a server socket
                //* if it's then accept it || if not then it's a client and you must receive the request
                struct sockaddr_in clientAddress;
                socklen_t addrlen =  sizeof(clientAddress);

                int connectSD = accept( (*readyToConnect) , (struct sockaddr *)&clientAddress , &addrlen);
                if (connectSD == -1 ) {
                    continue ;
                }
                //clientSocketsVector.push_back(connectSD);
                FD_SET(connectSD, &allsd);
                if (connectSD > max) {
                    max = connectSD ;
                }

            } else {
                // recev 

                // socket_iterator checkValidClientConnect = std::find(clientSocketsVector.begin(), clientSocketsVector.end(), i);
                // if (checkValidClientConnect != clientSocketsVector.end()) {
                //     write(0, "Valid Client\n", (16));
                // } else {
                //     write(0, "Invalid Client\n", (16));
                // }

                int recevRequestLen = recv(i , buffer, sizeof(buffer), 0);
                //! clientsMap[i] = buffer;

                std::string convert(buffer);
                std::string request = parseRequestPerBuffer(convert, requestOutput);
                clientSocketsMap[i] = request;
                
                FD_CLR(i, &readsd);

                FD_SET(i, &writesd);
                if (recevRequestLen < 0) {
                    std::cerr << "Error: recev() " << std::endl;
                } else {
                    //send
                    std::string res = "HTTP/1.1 200 OK\r\n";
                    res += "Content-Length: 13\r\n";
                    res += "Content-Type: text/html; charset=UTF-8\r\n";
                    res += "\r\n";
                    res += requestOutput;
                    // res += "Hello world \r\n";
                    // write(0, buffer, strlen(buffer)); // this line is responsible for outputing the request in stdin

                    // if ( FD_ISSET(i, &writesd) && send(i, res.c_str(), res.length(), 0) < 0 ) {
                    //     std::cerr << "Error : send() " << std::endl;
                    // }
                    // write(0, res.c_str(), strlen(res.c_str()));

                    //? send here the response based upon the parseRequestPerBuffer
                    if ( FD_ISSET(i, &writesd) && send(i, res.c_str(), res.length(), 0) < 0 ) {
                        std::cerr << "Error : send() " << std::endl;
                    }

                }
                close(i);
                //FD_CLR(i, &allsd);
                
            }
        }
    }
}

//* USE then SELECT()
//* ACCEPT && GET A NEW SOCKETDESCR FOR SEND AND RECEIVE
