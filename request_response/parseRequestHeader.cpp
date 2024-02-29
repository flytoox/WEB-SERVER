#include "../includes/webserve.hpp"

static std::vector<std::string> splitBySpace(Request &request, const std::string& httpLine ) {

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(httpLine);

    while (std::getline(tokenStream, token, ' ')) {
        if (token.length())
            tokens.push_back(token);
    }

    token = tokens[0];

    //* Whitespaces ruled : https://www.rfc-editor.org/rfc/rfc7230#section-3.2

    if ( token.back() == ' ' ) {

        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ; 
    }

    return tokens;
}

static void parseSingleLine(std::vector<std::string> &headerSplitVector, Request &request) {

    int index = -1;

    if (headerSplitVector.size() != 3) {
    
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>"); 
        throw "400";
    
    }

    for (const_vector_it it = headerSplitVector.begin() ; it != headerSplitVector.end(); ++it) {

        std::string value = (*it); std::string response;
        ++index;

        if (index == 0) {
            if ( request.getHttpVerb().empty() && ( (value == "GET") || (value == "POST") || (value == "DELETE")) ) {
                request.setHttpVerb(value);
            } else {
                request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody("<html><h1>400 Bad Request</h1></html>");
                throw "400" ; 
            }
        }

        if (index == 1) {
            if ( (request.getUri()).empty() && value[0] == '/') {
                request.setUri(value);
            } else {
                request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody("<html><h1>400 Bad Request</h1></html>");
                throw "400" ;  
            }
        }

        if (index == 2) {
            if ( (request.getHTTPVersion()).empty() && (value == "HTTP/1.1")) {
                request.setHTTPVersion(value);
            } else {
                request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody("<html><h1>400 Bad Request</h1></html>");
                throw "400" ;     
            }
        }
    }
}


static void parseHost(std::vector<std::string> &headerSplitVector, Request &request) {

    if  (headerSplitVector.size() != 2) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ; 
    }

    if (request.getHttpRequestHeaders().find("Host:") != request.getHttpRequestHeaders().end()) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ;     
    }

    std::size_t pos = headerSplitVector[1].find(":");
    std::string headerValue = headerSplitVector[1].substr(0, pos);

    pair hostPair = std::make_pair("Host:", headerValue);
    request.setHttpRequestHeaders(hostPair);

    if (request.getHttpRequestHeaders().find("Port:") != request.getHttpRequestHeaders().end()) {
    
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ;    
    }

    std::string httpPort = headerSplitVector[1].erase(0, ++pos);
    //int port = std::atoi(httpPort.c_str());

    pair portPair = std::make_pair(std::string("Port:"), httpPort);
    request.setHttpRequestHeaders(portPair);

}

static void parseContentType(std::vector<std::string> &headerSplitVector, Request &request) {


    if  (headerSplitVector.size() == 1) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ; 
    }

    if (request.getHttpRequestHeaders().find("Content-Type:") != request.getHttpRequestHeaders().end()) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ;      
    }


    std::string headerValue = headerSplitVector[1];
    pair contentType = std::make_pair(std::string("Content-Type:"), headerValue);
    if (headerSplitVector[1] == "multipart/form-data;") {
        std::string boun = headerSplitVector[2].substr(headerSplitVector[2].find('=') + 1);
        boun.insert(0, "--");
        request.setBoundary(boun);
    }
    request.setHttpRequestHeaders(contentType);    

}

static void parseContentLength(std::vector<std::string> &headerSplitVector, Request &request) {

    if  (headerSplitVector.size() != 2) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ; 
    }

    if (request.getHttpRequestHeaders().find("Content-Length:") != request.getHttpRequestHeaders().end()) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ;    
    }


    std::string headerValue = headerSplitVector[1];
    pair contentLength = std::make_pair(std::string("Content-Length:"), headerValue);
    request.setHttpRequestHeaders(contentLength);    

}

static void parseTransferEncoding(std::vector<std::string> &headerSplitVector, Request &request) {

    std::string response = "";

    if  (headerSplitVector.size() != 2) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ; 
    }
   

    if (request.getHttpRequestHeaders().find("Transfer-Encoding:") != request.getHttpRequestHeaders().end()) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><h1>400 Bad Request</h1></html>");
        throw "400" ;    
    }


    std::string transferEncoding = headerSplitVector[1];
    pair contentLength = std::make_pair(std::string("Transfer-Encoding:"), transferEncoding);
    request.setHttpRequestHeaders(contentLength);  

}

void tokenizeHttpHeader(std::vector<std::string> &headerSplitVector, Request &request) {

    std::string track = headerSplitVector[0];
    if ( ( track.find("GET") != std::string::npos ) || 
        ( track.find("POST") != std::string::npos ) || 
        ( track.find("DELETE") != std::string::npos ) )
        parseSingleLine(headerSplitVector, request);
    if (request.getHttpVerb().empty()) {
            request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody("<html><h1>400 Bad Request</h1></html>");
            throw "400" ;
    }
    if (track == "Host:" )
        parseHost(headerSplitVector, request);
    if (track == "Content-Type:" )
        parseContentType(headerSplitVector, request);
    if (track == "Content-Length:" )
        parseContentLength(headerSplitVector, request);
    if (track == "Transfer-Encoding:" )
        parseTransferEncoding(headerSplitVector, request);
    return ;

}

void parseAndSetRequestHeader(Request &request) {

    std::string header = request.getRequestHeader();
    std::string httpRequestHeader = "";
    std::vector<std::string> spaceParse;

    std::size_t headerBoundary = header.find("\r\n\r\n");
    header = header.substr(0, headerBoundary + 4);

    std::string body = request.getRequestHeader();
    std::string restToRequestBody = body.substr(headerBoundary + 4);
    request.setRequestBody(restToRequestBody);


    while ( header.length() != 2 ) {

        std::size_t found = header.find("\r\n");

        if ( found != std::string::npos ) {

            httpRequestHeader = header.substr(0, found);
            //std::cout << "|" << httpRequestHeader;
            spaceParse = splitBySpace(request, httpRequestHeader);

            tokenizeHttpHeader(spaceParse, request);

            header.erase(0, found + 2);
        } else {
            //exit (0);
            if ( request.getHttpVerb().empty() ) {
                request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody("<html><h1>400 Bad Request</h1></html>");
                throw "400" ; 
            }
            break ;
        }
    }

    validateRequest(request);

}