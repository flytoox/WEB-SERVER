#include "../includes/webserve.hpp"


static void push_convert(std::string &convert, char *buffer, int r) {
    for (int i = 0; i != r; i++)
        convert.push_back(buffer[i]);
}

void reCheckTheServer(configFile &configurationServers, std::string &header, Request &request) {

    try {
        Server serverReform;
        std::string v1 = header.substr(header.find("Host: "));
        std::string hostHeader = v1.substr(0, v1.find("\n"));
        std::string hostValue = hostHeader.substr(hostHeader.find(" ") + 1);
        hostValue.erase(hostValue.length() - 1);

        if ( request.dup == true ) {

            for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); ++it) {
                std::map<std::string, std::string>tmp = it->getdirectives();
                if (tmp["server_name"] == hostValue && tmp["listen"] == request.RePort && tmp["host"] == request.ReHost ) {
                    serverReform = *it;
                    std::map<std::string, std::string> serverDirectives = serverReform.getdirectives();
                    std::vector<std::map<std::string, std::string> > serverLocationsBlock = serverReform.getlocationsBlock();
                    request.setDirectivesAndPages(serverDirectives, serverReform.getPages());
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

void receiveRequestPerBuffer(std::map<int, Request> &simultaneousRequests, int &i, configFile &configurationServers, fd_set &allsd) {

    char buffer[1024] = {0};
    std::string res;
    int recevRequestLen = recv(i , buffer, 1024, 0);
    if (recevRequestLen < 0) {
        std::cerr << "Error: recv(): " << strerror(errno) << std::endl;
        close(i), FD_CLR(i, &allsd); ++i; return ;
    }

    std::string convert;
    push_convert(convert, buffer, recevRequestLen);

    if ( ! (simultaneousRequests[i].getRequestBodyChunk()) ) {
        //! REQUEST HEADER
        simultaneousRequests[i].setRequestHeader(convert);
        if ( ((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") != std::string::npos ) {
                std::string header = (simultaneousRequests[i]).getRequestHeader();
                if ((simultaneousRequests[i]).reCheck != true) {
                    (simultaneousRequests[i]).reCheck = true;
                    reCheckTheServer(configurationServers, header, simultaneousRequests[i]);
                }
                parseAndSetRequestHeader(simultaneousRequests[i]);
                (simultaneousRequests[i].setRequestBodyChunk(true));
                (simultaneousRequests[i]).reachedBodyLength = (simultaneousRequests[i].getRequestBody()).length();
                if ((simultaneousRequests[i]).reachedBodyLength >= (simultaneousRequests[i]).realContentLength) {
                    parseRequestBody(simultaneousRequests[i]);
                    checkRequestedHttpMethod(simultaneousRequests[i]);
                }
        }
        // else if (recevRequestLen < 1024 && ((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") == std::string::npos) {
        //     (simultaneousRequests[i]).response = responseBuilder()
        //         .addStatusLine("408")
        //         .addContentType("text/html")
        //         .addResponseBody(simultaneousRequests[i].getPageStatus(408));
        //         throw ("408");
        // }
    } else {
        //! REQUEST BODY
        simultaneousRequests[i].setRequestBody(convert);
        (simultaneousRequests[i]).reachedBodyLength = (simultaneousRequests[i].getRequestBody()).length();
        if (((simultaneousRequests[i]).reachedBodyLength >= (simultaneousRequests[i]).realContentLength) || \
            ((((simultaneousRequests[i]).getHttpRequestHeaders()).find("Transfer-Encoding:") != (simultaneousRequests[i]).getHttpRequestHeaders().end()) && \
                (simultaneousRequests[i].getRequestBody().find("0\r\n\r\n") != std::string::npos)) ) {
            parseRequestBody((simultaneousRequests[i]));
            checkRequestedHttpMethod(simultaneousRequests[i]);
        }
    }
}
