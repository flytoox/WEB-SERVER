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
        hostValue.erase(hostValue.size() - 1);

        if (request.dup == true) {

            for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); it++) {
                std::map<std::string, std::string>tmp = it->getdirectives();
                if (tmp["server_name"] == hostValue && tmp["listen"] == request.RePort && tmp["host"] == request.ReHost) {
                    serverReform = *it;
                    std::map<std::string, std::string> serverDirectives = serverReform.getdirectives();
                    std::vector<std::map<std::string, std::string> > serverLocationsBlock = serverReform.getlocationsBlock();
                    request.setDirectivesAndPages(serverDirectives, serverReform.getPages());
                    request.setLocationsBlock(serverLocationsBlock);
                    break ;
                }
            }
        }
    } catch (std::exception &e) {
        request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody("<html><h1>400 Bad Request22</h1></html>");
        throw "400";
        std::cout << e.what() << std::endl;
    }

}


std::vector<std::string> customSplitRequest(const std::string& input, const std::string& delimiter) {
    std::vector<std::string> result;
    std::size_t start = 0;
    std::size_t end = input.find(delimiter);
    while (end != std::string::npos) {
        end += delimiter.length();
        result.push_back(input.substr(start, end - start));
        start = end;
        end = input.find(delimiter, start);
    }

    result.push_back(input.substr(start));
    return result;
}

bool parseFirstLine(std::string &s, Request &request) {
    std::vector<std::string> lines = splitWhiteSpaces(s);
    if (lines.size() != 3 || lines[2] != "HTTP/1.1\r\n" || lines[1][0] != '/') {
        return false;
    }
    for (size_t i = 0; i < lines[0].length(); i++) {
        if (!isalpha(lines[0][i]) && !isupper(lines[0][i])) {
            return false;
        }
    }
    request.setHttpVerb(lines[0]);
    request.setUri(lines[1]);
    request.setHTTPVersion(lines[2].erase(lines[2].length() - 2));
    return true;
}

bool parseDefaultLine(std::string &s, Request &request) {
    std::vector<std::string> v = splitWithChar(s, ':');
    if (v[0][v[0].length() - 1] == ' ')
        return false;
    int l = 0;
    while (v[1][l++] == ' ');
    v[1].erase(0, l);
    if ((v[0] == "Content-Length" || v[0] == "Host" || v[0] == "Transfer-Encoding") && request.getHttpRequestHeaders().count(v[0])) 
        return false;
    std::string value = "";
    for (size_t i = 1; i < v.size(); i++) {
        value += v[i];
        if (i < (s[s.length()-1] == ':' ? v.size() : v.size() - 1))
            value += ":";
    }
    request.setHttpRequestHeaders(make_pair(v[0], value));
}

bool parseBody(std::string &s, Request &request) {
    if (request.getHttpRequestHeaders().find("Content-Length") != request.getHttpRequestHeaders().end()) {
        std::string contentLength = request.getHttpRequestHeaders()["Content-Length"];
        if (contentLength.find_first_not_of("0123456789") != std::string::npos)
            return true;
        request.setRequestBody(s);
        if (request.getRequestBody().length() >= std::stoi(contentLength)) {
            request.setRequestBodyChunk(false);
            return false;
        }
    } else if (request.getHttpRequestHeaders().find("Transfer-Encoding") != request.getHttpRequestHeaders().end()) {
        if (s.find("0\r\n\r\n") != std::string::npos) {
            request.setRequestBody(s);
            request.setRequestBodyChunk(false);
            return false;
        }
    }
    return true;
}

bool parseHeader(std::string &s, Request &request) {
    if (request.getRequestBodyChunk())
        return parseBody(s, request);
    std::vector<std::string> lines = customSplitRequest(s, "\r\n");
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i] == "\r\n" && !request.getHttpVerb().empty()) {
            validateHeader(request);
            request.setRequestBodyChunk(true);
        } else if (lines[i] == "\r\n") {
            continue;
        }
        if (lines[i].find("\r\n") == std::string::npos) {
            s = lines[i];
            return false;
        }
        if ((request.getHttpVerb().empty() && !parseFirstLine(lines[i], request)) || parseDefaultLine(lines[i], request))
            return true;
    }
}

void receiveRequestPerBuffer(std::map<int, Request> &simultaneousRequests, int i, configFile &configurationServers, fd_set &allsd) {

    char buffer[1024] = {0};
    std::string res;
    int recevRequestLen = recv(i , buffer, 1024, 0);
    if (recevRequestLen < 0) {
        std::cerr << "Error: recv(): " << strerror(errno) << std::endl;
        close(i), FD_CLR(i, &allsd); return ;
    }

    std::string convert;
    push_convert(convert, buffer, recevRequestLen);

    simultaneousRequests[i].stringUnparsed += convert;
    if (parseHeader(simultaneousRequests[i].stringUnparsed, simultaneousRequests[i])) {
        simultaneousRequests[i].response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody(simultaneousRequests[i].getPageStatus(400));
        throw "400";
    }
    // if (!(simultaneousRequests[i].getRequestBodyChunk())) {
    //     //! REQUEST HEADER


    //     if (((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") != std::string::npos) {
    //         std::string header = simultaneousRequests[i].getRequestHeader();
    //         if ((simultaneousRequests[i]).reCheck != true) {
    //             (simultaneousRequests[i]).reCheck = true;
    //             reCheckTheServer(configurationServers, header, simultaneousRequests[i]);
    //         }
    //         parseAndSetRequestHeader(simultaneousRequests[i]);
    //         validateHeader(request);
    //     }
    //     // else if (recevRequestLen < 1024 && ((simultaneousRequests[i]).getRequestHeader()).find("\r\n\r\n") == std::string::npos) {
    //     //     (simultaneousRequests[i]).response = responseBuilder()
    //     //         .addStatusLine("408")
    //     //         .addContentType("text/html")
    //     //         .addResponseBody(simultaneousRequests[i].getPageStatus(408));
    //     //         throw ("408");
    //     // }
    // } else {
    //     //! REQUEST BODY
    //     simultaneousRequests[i].setRequestBody(convert);
    //     (simultaneousRequests[i]).reachedBodyLength = (simultaneousRequests[i].getRequestBody()).length();
    //     if (((simultaneousRequests[i]).reachedBodyLength >= (simultaneousRequests[i]).realContentLength) || \
    //         ((((simultaneousRequests[i]).getHttpRequestHeaders()).find("Transfer-Encoding") != (simultaneousRequests[i]).getHttpRequestHeaders().end()) && \
    //             (simultaneousRequests[i].getRequestBody().find("0\r\n\r\n") != std::string::npos)) ) {
    //         parseHeaderBody((simultaneousRequests[i]));
    //         checkRequestedHttpMethod(simultaneousRequests[i]);
    //     }
    // }


}
