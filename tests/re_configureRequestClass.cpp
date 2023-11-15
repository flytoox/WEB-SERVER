#include "webserve.hpp"

//! throw std::runtime_error(msg);


enum Headers {
    SINGLE_LINE = 1,
    CONTENT_T,
    CONTENT_L,
    TRANSFER_ENC,
    CONNECTION,
};

int tokenizeHttpHeader(std::string &httpHeader) {
    
    std::string track;

    track = httpHeader.substr(0, 18);

    if ( track.find("GET ") || track.find("POST ") || track.find("DELETE ") )
        return 1;
    if (track.find("Content-Type: "))
        return 2;
    if (track.find("Content-Length: "))
        return 3;
    if (track.find("Transfer-Encoding: "))
        return 4; 
    if (track.find("Connection: "))
        return 5;
}


void parseSingleLine(std::string &httpHeader, Request &request) {

    std::size_t spaceStartPosition = 0;
    std::size_t spaceEndPosition = httpHeader.find(" ");

    std::string toParse = httpHeader.substr(spaceStartPosition, spaceEndPosition);
    if (toParse == "GET" || toParse == "POST" || toParse == "DELETE") {
        request.setHttpVerb(toParse);
    } else {
        throw std::runtime_error("400 Bad Request");
    }


    spaceStartPosition = ++spaceEndPosition;
    spaceEndPosition += httpHeader.find(" ");

    // spaceStartPosition = spaceEndPosition + toParse.length();
    // spaceEndPosition = httpHeader.find(" ");

    std::string toParse = httpHeader.substr(spaceStartPosition, spaceEndPosition);
    request.setUri(toParse);

}

void parseHost(std::string &httpHeader, Request &request) {

    std::string header = httpHeader.substr(0, 5);
    if (header != "Host: ") {
        throw std::runtime_error("400 Bad Request");
    }

    std::string headerValue = httpHeader.substr(6);
    request.setHost(headerValue);

}

void parseContentType(std::string &httpHeader, Request &request) {

    std::string header = httpHeader.substr(0, 13);
    if (header != "Content-Type: ") {
        throw std::runtime_error("400 Bad Request");
    }

    std::string headerValue = httpHeader.substr(14);
    request.setContentType(headerValue);
}

void parseContentLength(std::string &httpHeader, Request &request) {

    std::string header = httpHeader.substr(0, 15);
    if (header != "Content-Length: ") {
        throw std::runtime_error("400 Bad Request");
    }

    std::string headerValue = httpHeader.substr(16);
    request.setContentLength(headerValue);
}

void parseTransferEncoding(std::string &httpHeader, Request &request) {

    std::string header = httpHeader.substr(0, 18);
    if (header != "Transfer-Encoding: ") {
        throw std::runtime_error("400 Bad Request");
    }

    std::string headerValue = httpHeader.substr(19);
    request.setTransferEncoding(headerValue);
}

void parseConnection(std::string &httpHeader, Request &request) {

    std::string header = httpHeader.substr(0, 11);
    if (header != "Connection: ") {
        throw std::runtime_error("400 Bad Request");
    }

    std::string headerValue = httpHeader.substr(12);
    request.setConnection(httpHeader); 
}


void configureTheHttpHeaderRequestClass(std::string &header, Request &request) {

    std::stringstream stream(header);
    std::string httpHeader;

    while ( ! stream.eof() ) {
        //std::getline(stream, httpHeader, '\r');
        std::size_t spaceStartPosition = 0;
        std::size_t spaceEndPosition = httpHeader.find("\r\n\r\n");

        std::string httpMethod = httpHeader.substr(spaceStartPosition, spaceEndPosition);
        if (httpMethod == "GET" || httpMethod == "POST" || httpMethod == "DELETE") {
            request.setHttpVerb(httpMethod);
        } else {
            throw std::runtime_error("400 Bad Request");
        }

        int level = tokenizeHttpHeader(httpHeader);


        switch(level) {

            case(1) : parseSingleLine(httpHeader, request); break ;
            case(2) : parseContentType(httpHeader, request); break ;
            case(3) : parseContentLength(httpHeader, request); break ;
            case(4) : parseTransferEncoding(httpHeader, request); break ;
            case(5) : parseConnection(httpHeader, request); break ;
            case(6) : break ;
            default:
                continue ;
        }
    }
}



std::string &parseRequestPerBuffer(std::string &buffer, std::string &requestOutput) {

    Request request;
    std::string allTheRequest;

    bool moveToHTTPBody = false;

    std::string header; std::string body;


    try {
        while ( ! moveToHTTPBody ) {

            header.append(buffer);

            if ( header.find("\r\n\r\n") ) {
                configureTheHttpHeaderRequestClass(header, request);
                checkRequestFormat(request, requestOutput);
                moveToHTTPBody = true;
            }
        }


        // if ( moveToHTTPBody ) {
        //     body.append(buffer);
        //     if (! buffer.length() ) {
        //         parseTheHttpBody(header, request);
        //     }
        // }

        allTheRequest.append(buffer);

    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return (allTheRequest);
}
