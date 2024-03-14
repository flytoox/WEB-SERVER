#include "../includes/webserve.hpp"

configFile configurationServers;

// static void push_convert(std::string &convert, char *buffer, int r) {
//     for (int i = 0; i != r; i++)
//         convert.push_back(buffer[i]);
    
// }

void reCheckTheServer(configFile &configurationServers, std::string &hostValue, Request &request) {
    try {
        if (request.dup == true) {
            for (const_iterator it = (configurationServers.getServers()).begin(); it != (configurationServers.getServers()).end(); it++) {
                std::map<std::string, std::string>tmp = it->getdirectives();
                if (tmp["server_name"] == hostValue && tmp["listen"] == request.RePort && tmp["host"] == request.ReHost) {
                    std::map<std::string, std::string> serverDirectives = it->getdirectives();
                    std::vector<std::map<std::string, std::string> > serverLocationsBlock = it->getlocationsBlock();
                    request.setDirectivesAndPages(serverDirectives, it->getPages());
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

void    parseUri(std::string &uri) {
    std::vector<std::string> v = splitWithChar(uri, '/');
    std::stack<std::string> s;
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i] == "..") {
            if (!s.empty())
                s.pop();
        } else if (v[i] != ".") {
            s.push(v[i]);
        }
    }
    uri = "";
    while (!s.empty()) {
        uri = s.top() + uri;
        s.pop();
        if (!s.empty())
            uri = "/" + uri;
    }
    if (uri.empty()) uri = "/";
}

bool parseFirstLine(std::string &s, Request &request) {
    std::vector<std::string> lines = splitWhiteSpaces(s);
    if (lines.size() != 3 || lines[2] != "HTTP/1.1" || lines[1][0] != '/') {
        std::cerr << "Error: HTTP" << std::endl;
        return false;
    }
    for (size_t i = 0; i < lines[0].length(); i++) {
        if (!isalpha(lines[0][i]) && !isupper(lines[0][i]))
            return false;
    }
    parseUri(lines[1]);
    request.setHttpVerb(lines[0]);
    request.setUri(lines[1]);
    request.setHTTPVersion(lines[2]);
    return true;
}

bool parseDefaultLine(std::string &s, Request &request) {
    std::vector<std::string> v = splitWithChar(s, ':');
    if (v.size() < 2)
        return false;
    if (v[0][v[0].length() - 1] == ' ')
        return false;
    int l = -1;
    while (v[1][++l] == ' ');
    v[1].erase(0, l);
    if ((v[0] == "Content-Length" || v[0] == "Host" || v[0] == "Transfer-Encoding") && request.getHttpRequestHeaders().count(v[0]))
        return false;
    std::string value = "";
    for (size_t i = 1; i < v.size(); i++) {
        value += v[i];
        if (i < (s[s.length()-1] == ':' ? v.size() : v.size() - 1))
            value += ":";
    }
    request.setHttpRequestHeaders(make_pair(v[0], value.erase(value.length() - 2)));
    return true;
}

size_t custAtoi(const std::string &s) {
    size_t res = 0;
    for (size_t i = 0; i < s.length(); i++) {
        res = res * 10 + s[i] - '0';
    }
    return res;
}

bool parseBody(Request &request) {
    request.setRequestBody(request.stringUnparsed);
    request.stringUnparsed = "";
    if (request.reCheck != true) {
        request.reCheck = true;
        reCheckTheServer(configurationServers, request.getHttpRequestHeaders()["Host"], request);
    }
    mapConstIterator MaxBodySize = request.getLocationBlockWillBeUsed().find("client_max_body_size");
    if (MaxBodySize != request.getLocationBlockWillBeUsed().end()) {
        size_t sizeMax = custAtoi(MaxBodySize->second);
        if (request.getRequestBody().length() > sizeMax) {
            request.response = responseBuilder()
            .addStatusLine("413")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(413));
            throw "413";
        }
    }
    return false;
}

bool parseHeader(std::string &s, Request &request) {
    if (request.getRequestBodyChunk())
        return parseBody(request);
    std::vector<std::string> lines = customSplitRequest(s, "\r\n");
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i] == "\r\n" && !request.getHttpVerb().empty()) {
            validateHeader(request);
            request.setRequestBodyChunk(true);
            s = "";
            for (size_t j = i + 1; j < lines.size(); j++)
                s += lines[j];
            if (request.getHttpRequestHeaders().count("Content-Length")) {
                if (!checkOverFlow(request.getHttpRequestHeaders()["Content-Length"])) {
                    request.response = responseBuilder()
                        .addStatusLine("413")
                        .addContentType("text/html")
                        .addResponseBody(request.getPageStatus(413));
                    throw "413";
                }
                request.realContentLength = custAtoi(request.getHttpRequestHeaders()["Content-Length"]);
            } else request.realContentLength = 0;
            std::cerr << "Content-Length: " << request.realContentLength << std::endl;
            return parseBody(request);
        } else if (lines[i] == "\r\n") {
            continue;
        }
        if (lines[i].find("\r\n") == std::string::npos) {
            s = lines[i];
            return false;
        }
        if ((!request.getHttpVerb().empty() && !parseDefaultLine(lines[i], request)) || (request.getHttpVerb().empty() && !parseFirstLine(lines[i], request))) {
            return true;
        }
    }
    return false;
}

void receiveRequestPerBuffer(std::map<int, Request> &simultaneousRequests, int i, configFile &cnf, fd_set &allsd) {
    configurationServers = cnf;
    std::string res;
    int recevRequestLen = recv(i , simultaneousRequests[i].buffer, 10240, 0);
    if (recevRequestLen < 0) {
        std::cerr << "Error: recv(): " << strerror(errno) << std::endl;
        close(i), FD_CLR(i, &allsd); return ;
    }
    if (recevRequestLen) {
        simultaneousRequests[i].isTimeOut = false;
        simultaneousRequests[i].setTimeout();
    }
    simultaneousRequests[i].stringUnparsed.append(simultaneousRequests[i].buffer, recevRequestLen);
    if (parseHeader(simultaneousRequests[i].stringUnparsed, simultaneousRequests[i])) {
        simultaneousRequests[i].response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody(simultaneousRequests[i].getPageStatus(400));
        throw "400";
    }
    const std::string &body = simultaneousRequests[i].getRequestBody();
    bool isTransferEncoding = simultaneousRequests[i].getHttpRequestHeaders().count("Transfer-Encoding");
    if (simultaneousRequests[i].getRequestBodyChunk() && (body.length() >= simultaneousRequests[i].realContentLength || (isTransferEncoding && body.find("0\r\n\r\n") != std::string::npos))) {
        parseRequestBody((simultaneousRequests[i]));
        simultaneousRequests[i].stringUnparsed = "";
        checkRequestedHttpMethod(simultaneousRequests[i]);
    }
}
