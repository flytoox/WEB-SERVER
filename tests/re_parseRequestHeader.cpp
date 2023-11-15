#include "webserve.hpp"

enum Headers {
    HOST,
    SINGLE_LINE,
    CONTENT_T,
    CONTENT_L,
    TRANSFER_ENC,
    CONNECTION,
};

int tokenizeHttpHeader(std::string &header) {

    std::string track = header.substr(0, 18);

    //std::cout << "c\n";
    //std::cout << "PA: " << track << std::endl;
    if (track.find("Host: ") != std::string::npos )
    {
        return 0;
    }
    if ( (track.find("GET ") != std::string::npos ) || (track.find("POST ") != std::string::npos ) || (track.find("DELETE ") != std::string::npos ))
        return 1;
    if (track.find("Content-Type: ") != std::string::npos )
        return 2;
    if (track.find("Content-Length: ") != std::string::npos )
        return 3;
    if (track.find("Transfer-Encoding: ") != std::string::npos )
        return 4; 
    if (track.find("Connection: ") != std::string::npos )
        return 5;
    return (-1);    

}

void parseSingleLine(std::string &httpHeader, Request &request) {

    std::size_t spaceStartPosition = 0;
    std::size_t spaceEndPosition = httpHeader.find(" ");

    std::string toParse = httpHeader.substr(spaceStartPosition, spaceEndPosition);
    if (toParse == "GET" || toParse == "POST" || toParse == "DELETE") {
        request.setHttpVerb(toParse);
    } else {
        //throw std::runtime_error("400 Bad Request");
        std::string msg = "1/400 Bad Request";
        request.setrequestOutputTest(msg);
    }

    httpHeader.erase(0, ++spaceEndPosition); spaceStartPosition = 0; spaceEndPosition = httpHeader.find(" ");

    toParse = httpHeader.substr(spaceStartPosition, spaceEndPosition);
    request.setUri(toParse);

    httpHeader.erase(0, ++spaceEndPosition); spaceStartPosition = 0; spaceEndPosition = httpHeader.length() - 1;

    toParse = httpHeader.substr(spaceStartPosition, spaceEndPosition);
    if (toParse != "HTTP/1.1") {
        //throw std::runtime_error("400 Bad Request");
        std::string msg = "2/ 400 Bad Request";
        request.setrequestOutputTest(msg);
    }

}

void parseHost(std::string &httpHeader, Request &request) {

    std::string host = httpHeader.substr(0, 6);

    if (host != "Host: ") {
        //throw std::runtime_error("400 Bad Request");
        std::string msg = "3/ 400 Bad Request";
        request.setrequestOutputTest(msg);
    }

    httpHeader = httpHeader.erase(0, 6);

    std::size_t pos = httpHeader.find(":");
    std::string headerValue = httpHeader.substr(0, pos);
    request.setHost(headerValue);


    std::string httpPort = httpHeader.erase(0, ++pos);
    int port = std::atoi(httpPort.c_str());
    request.setPort(port);
}

void noNeedparse(void) {
    return ;
}

void parseContentType(std::string &httpHeader, Request &request) {

    std::string header = httpHeader.substr(0, 14);

    if (header != "Content-Type: ") {
        //throw std::runtime_error("400 Bad Request");
        std::string msg = "4/ 400 Bad Request";
        request.setrequestOutputTest(msg);
    }

    std::string headerValue = httpHeader.substr(14);
    request.setContentType(headerValue);
}

void parseContentLength(std::string &httpHeader, Request &request) {


    std::cout << "1 -- " << std::endl;
    std::cout << httpHeader << std::endl;
    std::cout << " -- " << std::endl;

    std::string header = httpHeader.substr(0, 16);
    if (header != "Content-Length: ") {
        //throw std::runtime_error("400 Bad Request");
        std::string msg = "5/ 400 Bad Request";
        request.setrequestOutputTest(msg);
    }

    std::string headerValue = httpHeader.substr(16);

    int replace = std::atoi(headerValue.c_str());
    request.setContentLength(replace);
}

void parseTransferEncoding(std::string &httpHeader, Request &request) {

    std::string header = httpHeader.substr(0, 19);
    if (header != "Transfer-Encoding: ") {
        //throw std::runtime_error("400 Bad Request");
        std::string msg = "6/ 400 Bad Request";
        request.setrequestOutputTest(msg);
    }

    std::string headerValue = httpHeader.substr(19);
    request.setTransferEncoding(headerValue);
}

void parseConnection(std::string &httpHeader, Request &request) {

    std::string header = httpHeader.substr(0, 12);
    if (header != "Connection: ") {
        //throw std::runtime_error("400 Bad Request");
        std::string msg = "7/ 400 Bad Request";
        request.setrequestOutputTest(msg);
    }

    std::string headerValue = httpHeader.substr(12);
    request.setConnection(httpHeader); 
}

void parseAndSetRequestHeader(Request &request) {




    std::string header = request.getRequestHeader();

    //std::stringstream httpHeader(header);
    std::string httpRequestHeader = "";

    //std::cout << "before: " << &header << std::endl;

    while ( header.length() != 0 ) {


        // std::cout << "after: " << &header << std::endl;
        // std::cout << header;
        //std::cout << request.getRequestHeader() << std::endl;
        //std::cout << header.length() << std::endl;
        //exit (0);
        //std::cout << "found: " << (request.getRequestHeader()).length() << std::endl;

        std::size_t found = header.find("\r\n");

       if ( found ) {
            httpRequestHeader = header.substr(0, found);
            std::cout << "pr: " << httpRequestHeader[found] << std::endl;
            std::cout << "pr - 1: " << httpRequestHeader[found - 1] << std::endl;
            // if (httpRequestHeader[httpRequestHeader.length() - 1] == '\r')
            //     std::cout << "\r is in";
            // else
            //     std::cout << "\r not";
            int level;
            level = tokenizeHttpHeader(httpRequestHeader);
            
            std::cout << "BEFORE: track: " << level << "| header: |" << httpRequestHeader;

            switch (level) {
                
                case 0 : parseHost(httpRequestHeader, request); break ;
                case 1 : parseSingleLine(httpRequestHeader, request); break ;
                case 2 : parseContentType(httpRequestHeader, request); break ;
                case 3 : parseContentLength(httpRequestHeader, request); break ;
                case 4 : parseTransferEncoding(httpRequestHeader, request); break ;
                case 5 : parseConnection(httpRequestHeader, request); break ;
                case 6 : noNeedparse() ; break ;
                // default:
                //     //throw std::runtime_error("400 Bad Request");
                //     std::string msg = "400 Bad Request"; request.setrequestOutputTest(msg);
            }

            //std::cout << "AFTER: track: " << level << "| header: |" << httpRequestHeader;
            header.erase(0, found + 2);

        } else {
            //header.erase(0, found + 2); continue ;
        }


        //std::cout<< "after: |" << httpRequestHeader << "|";

        // std::getline(httpHeader, httpRequestHeader, '\r');
        // std::cout << "before : |" << httpRequestHeader << "|";
        // if (httpRequestHeader.find('\n') != std::string::npos ) {
        //     httpRequestHeader.erase('\n');
        // }
        // std::cout << "after : |" << httpRequestHeader << "|";


        // std::cout << "stringstream: " << httpRequestHeader << std::endl;



    }
    
    validateTheRequestFormat(request);
    std::cout << "RE: " << request.getrequestOutputTest() << std::endl; exit (0);

}
