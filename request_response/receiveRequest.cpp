#include "../includes/webserve.hpp"


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
            .addResponseBody(request.getPageStatus(400));
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

void    parseUri( Request &request, std::string &uri) {
    if (uri.empty()) return;
    std::vector<std::string> v = splitWithChar(uri, '/');
    bool hasBs = (uri[uri.length() - 1] == '/');
    request.setSaveLastBS(hasBs);
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
        uri = "/" + s.top() + uri;
        s.pop();
    }
    if (uri.empty()) uri = "/";
    request.setUri(uri);
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
    parseUri(request, lines[1]);
    request.setHttpVerb(lines[0]);
    // request.setUri(lines[1]);
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

bool checkLimitRead(Request &request, size_t bodySize) {
    mapConstIterator MaxBodySize = request.getLocationBlockWillBeUsed().find("client_max_body_size");
    if (MaxBodySize != request.getLocationBlockWillBeUsed().end()) {
        size_t sizeMax = custAtoi(MaxBodySize->second);
        if (bodySize > sizeMax) {
            request.response = responseBuilder()
            .addStatusLine("413")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(413));
            throw "413";
        }
    }
    if (request.realContentLength < bodySize) {
        request.response = responseBuilder()
        .addStatusLine("413")
        .addContentType("text/html")
        .addResponseBody(request.getPageStatus(413));
        throw "413";
    }
    return false;
}

void writeOnFile(const std::string &filename, const std::string &content) {
    if (content.empty())
        return ;
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: file not open" << std::endl;
        return ;
    }
    file << content;
}

void    fillFirstPartOfMultipart(Request &request) {
    size_t pos = request.stringUnparsed.find("\r\n\r\n");
    if (pos == std::string::npos) {
        request.firstPart += request.stringUnparsed;
        request.stringUnparsed = "";
        return;
    }
    request.firstPart += request.stringUnparsed.substr(0, pos + 4);
    request.stringUnparsed.erase(0, pos + 4);
    std::string &s = request.firstPart;
    std::string tmp = request.getBoundary() + "\r\nContent-Disposition: form-data; name=\"";
    for (size_t i = 0; i < tmp.length(); i++) {
        if (s[i] != tmp[i]) {
            request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(400));
            throw "400";
        }
    }
    pos = s.find("\"; filename=\"");
    if (pos == std::string::npos) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody(request.getPageStatus(400));
        throw "400";
    }
    pos += 13;
    size_t pos2 = s.find("\"", pos);
    if (pos2 == std::string::npos) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody(request.getPageStatus(400));
        throw "400";
    }
    tmp = "\"\r\nContent-Type: ";
    request.fileName = request.getLocationBlockWillBeUsed().find("upload_store")->second + "/" + s.substr(pos, pos2 - pos);
    for (size_t i = 0;i < tmp.length(); i++) {
        if (tmp[i] != s[pos2 + i]) {
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody(request.getPageStatus(400));
            throw "400";
        }
    }
    pos = request.fileName.find_last_of(".");
    if (pos == std::string::npos)
        pos = request.fileName.length();
    while (std::ifstream(request.fileName.c_str()))
        request.fileName.insert(pos, "_");
}

bool isGoodFirstPart(std::string &s) {
    return (s.length() >= 4 && s.substr(s.length() - 4) == "\r\n\r\n");
}

void multipartBody(Request &request) {
    try {
        if (!isGoodFirstPart(request.firstPart)) {
            fillFirstPartOfMultipart(request);
        }
        const std::string  &boundary = request.getBoundary();
        if (request.stringUnparsed.empty())
            return;

        if (request.stringUnparsed.length() == boundary.length() + 6) {
            swap(request.lastBoundary, request.stringUnparsed);
        } else if (request.stringUnparsed.length() < boundary.length() + 6) {
            request.lastBoundary += request.stringUnparsed;
            if (request.lastBoundary.length() > boundary.length() + 6) {
                request.stringUnparsed = request.lastBoundary.substr(0, request.lastBoundary.length() - boundary.length() - 6);
                request.lastBoundary = request.lastBoundary.substr(request.lastBoundary.length() - boundary.length() - 6);
            } else
                request.stringUnparsed = "";
        } else {
            request.stringUnparsed.insert(0, request.lastBoundary);
            request.lastBoundary = request.stringUnparsed.substr(request.stringUnparsed.length() - boundary.length() - 6);
            request.stringUnparsed.resize(request.stringUnparsed.length() - boundary.length() - 6);
        }
        writeOnFile(request.fileName, request.stringUnparsed);
        request.stringUnparsed = "";
        if (request.binaryRead == request.realContentLength) {
            if (request.lastBoundary == "\r\n"+boundary+"--\r\n") {
                request.response = responseBuilder()
                    .addStatusLine("201")
                    .addContentType("text/html")
                    .addLocationFile(request.fileName)
                    .addResponseBody(request.getPageStatus(201));
                throw "201";
            }
            std::remove(request.fileName.c_str());
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody(request.getPageStatus(400));
            throw "400";
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

size_t hexToDec(const std::string &s, Request &request) {
    size_t res = 0;
    for (size_t i = 0; i < s.length(); i++) {
        if (isdigit(s[i]) || (s[i] >= 'a' && s[i] <= 'f'))
            res = res * 16 + (s[i] >= '0' && s[i] <= '9' ? s[i] - '0' : s[i] - 'a' + 10);
        else {
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody(request.getPageStatus(400));
            throw "400";
        }
    }
    return res;
}

void requestChunked(Request &request) {
    std::string tmpBody;
    std::string &s = request.stringUnparsed;
    std::string &chnkUnparsed = request.chunkedUnparsed;
    for (size_t i = 0; i < s.length();i++) {
        if (request.chunkSize == -1 && s[i] != '\r')
            chnkUnparsed += s[i];
        else if (request.chunkSize == -1 && s[i] == '\r') {
            request.chunkSize = hexToDec(chnkUnparsed, request) + 3;
            std::cerr << "CHUNKED: " << request.chunkSize << std::endl;
            chnkUnparsed = "";
        } else if (request.chunkSize != -1) {
            if (request.chunkSize == 0) {
                if ((chnkUnparsed.length() >= 3 && chnkUnparsed != "\n\r\n") || (chnkUnparsed == "\n\r\n" && request.binaryRead != request.realContentLength)) {
                    std::cerr << "Error: Chunked0" << std::endl;
                    request.response = responseBuilder()
                        .addStatusLine("400")
                        .addContentType("text/html")
                        .addResponseBody(request.getPageStatus(400));
                    throw "400";
                }
                chnkUnparsed += s[i];
                continue;
            }
            long long cnt  = 0;
            for (; cnt < request.chunkSize && i < s.length(); cnt++, i++)
                chnkUnparsed += s[i];
            if (cnt) i--;
            request.chunkSize -= cnt;
            if (request.chunkSize == 0) {
                if (chnkUnparsed[0] != '\n' || chnkUnparsed[chnkUnparsed.length() - 1] != '\n' || chnkUnparsed[chnkUnparsed.length() - 2] != '\r'){
                    std::cerr << "|" << chnkUnparsed <<"|" <<std::endl;
                    std::cerr << "Error: Chunked1" << std::endl;
                    request.response = responseBuilder()
                        .addStatusLine("400")
                        .addContentType("text/html")
                        .addResponseBody(request.getPageStatus(400));
                    throw "400";
                }
                request.chunkSize = -1;
                for(size_t j = 1; j < chnkUnparsed.length() - 2; j++)
                    tmpBody += chnkUnparsed[j];
                chnkUnparsed = "";
            } else if (request.chunkSize < 0) {
                    std::cerr << "|" << chnkUnparsed <<"|" <<std::endl;
                    std::cerr << "Error: Chuned2" << std::endl;
                request.response = responseBuilder()
                    .addStatusLine("400")
                    .addContentType("text/html")
                    .addResponseBody(request.getPageStatus(400));
                throw "400";
            }
        }
    }
    s = tmpBody;
}

bool parseBody(Request &request, configFile &configurationServers) {
    request.binaryRead += request.stringUnparsed.length();
    checkLimitRead(request, request.binaryRead);
    if (request.getHttpRequestHeaders()["Transfer-Encoding"] == "chunked")
        requestChunked(request);
    if (!request.getBoundary().empty())  {
        multipartBody(request);
        return false;
    }
    request.setRequestBody(request.stringUnparsed);
    request.stringUnparsed = "";
    if (request.reCheck != true) {
        request.reCheck = true;
        reCheckTheServer(configurationServers, request.getHttpRequestHeaders()["Host"], request);
    }
    return checkLimitRead(request, request.binaryRead);
}

bool parseHeader(std::string &s, Request &request, configFile &configurationServers) {
    if (request.getRequestBodyChunk())
        return parseBody(request, configurationServers);
    std::vector<std::string> lines = customSplitRequest(s, "\r\n");
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i] == "\r\n" && !request.getHttpVerb().empty()) {
            validateHeader(request);
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
            request.setRequestBodyChunk(true);
            return parseBody(request, configurationServers);
        } else if (lines[i] == "\r\n") {
            continue;
        }
        if (lines[i].find("\r\n") == std::string::npos) {
            s = lines[i];
            return false;
        }
        if ((!request.getHttpVerb().empty() && !parseDefaultLine(lines[i], request))
            || (request.getHttpVerb().empty() && !parseFirstLine(lines[i], request))) {
            return true;
        }
    }
    return false;
}

void receiveRequestPerBuffer(std::map<int, Request> &requests, int i, configFile &cnf, fd_set &allsd) {
    std::string res;
    int recevRequestLen = 0;
    recevRequestLen = recv(i , requests[i].buffer, 1024, 0);
    if (recevRequestLen < 0) {
        std::cerr << "Error: recv()" << std::endl;
        close(i), FD_CLR(i, &allsd); return ;
    }
    if (recevRequestLen) {
        requests[i].checkTimeout = true;
        requests[i].setTimeout();
    }

    requests[i].stringUnparsed.append(requests[i].buffer, recevRequestLen);
    if (parseHeader(requests[i].stringUnparsed, requests[i], cnf)) {
        requests[i].response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody(requests[i].getPageStatus(400));
        throw "400";
    }
    if (requests[i].getRequestBodyChunk() && requests[i].binaryRead == requests[i].realContentLength) {
        requests[i].checkTimeout = false;
        requests[i].stringUnparsed = "";
        checkRequestedHttpMethod(requests[i]);
    }
}
