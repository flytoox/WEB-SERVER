#include "../includes/webserve.hpp"

STATUS_CODE_ENUM defineStatusCode(const std::string &type) {

    if (type == "200")
        return OK;
    if (type == "201")
        return CREATED;
    if (type == "204")
        return NO_CONTENT;
    if (type == "301")
        return MOVED_PERMANETLY;
    if (type == "400")
        return BAD_REQUEST;
    if (type == "403")
        return FORBIDDEN;
    if (type == "404")
        return NOT_FOUND;
    if (type == "405")
        return METHOD_NOT_ALLOWED;
    if (type == "409")
        return CONFLICT;
    if (type == "413")
        return REQUEST_TOO_LARGE;
    if (type == "414")
        return URI_TOO_LONG;
    if (type == "500")
        return INTERNAL_SERVER_ERROR;
    if (type == "501")
        return NOT_IMPLEMENTED;    
    if (type == "302")
        return FOUND;
    if (type == "408")
        return TIMEOUT;
    return BAD_GATEWAY;
}

std::string defineMimeType(const std::string &type) {

    if (type == "css")
        return "text/css";
    if (type == "xml")
        return "text/xml";
    if (type == "jpg")
        return "image/jpeg";
    if (type == "jpeg")
        return "image/jpeg";
    if (type == "png")
        return "image/png";
    if (type == "webp")
        return "image/webp";
    if (type == "js")
        return "application/javascript";
    if (type == "zip")
        return "application/zip";
    if (type == "img")
        return "application/octet-stream";
    if (type == "mp3")
        return "audio/mpeg";
    if (type == "mp4")
        return "video/mp4";
    if (type == "webm")
        return "video/webm";
    if (type == "json")
        return "application/json";
    if (type == "pdf")
        return "application/pdf";
    if (type == "txt")
        return "text/plain";
    if (type == "html")
        return ("text/html");
    return "application/octet-stream";
}

void responseBuilder::defineStatusLine(const std::string &type) {

    std::string ret;

    switch (defineStatusCode(type)) {
        case 0 : ret = RESPONSE_OK;  break ;
        case 1 : ret = RESPONSE_CREATED;  break ;
        case 2 : ret = RESPONSE_NO_CONTENT;  break ;
        case 3 : ret = RESPONSE_MOVED_PERMANETLY;  break ;
        case 4 : ret = RESPONSE_BAD_REQUEST;  break ;
        case 5 : ret = RESPONSE_FORBIDDEN;  break ;
        case 6 : ret = RESPONSE_NOT_FOUND; break ;
        case 7 : ret = RESPONSE_METHOD_NOT_ALLOWED; break ;
        case 8 : ret = RESPONSE_CONFLICT; break ;
        case 9 : ret = RESPONSE_REQUEST_TOO_LARGE; break ;
        case 10 : ret = RESPONSE_URI_TOO_LONG; break ;
        case 11 : ret = RESPONSE_INTERNAL_SERVER_ERROR; break ;
        case 12 : ret = RESPONSE_NOT_IMPLEMENTED; break ;
        case 13 : ret = RESPONSE_BAD_GATEWAY; break ;
        case 14 : ret = RESPONSE_FOUND; break ;
        case 15 : ret = RESPONSE_REQUEST_TIMEOUT; break ;
    }

    resultMsg = ret;
}

void responseBuilder::defineContentType(const std::string &extension) {

    // res += extension == "mp4" ? "video/mp4" : "text/html";

    if (extension == "text/html"){
        headersResponses.insert(std::make_pair(CONTENT_TYPE, extension));
        return ;
    }
    std::string type;

    // std::cout << "ALL |" << extension << "|\n";

    size_t lastSlashPos = extension.find_last_of('.');

    // Extract the substring starting from the position after the last '/'
    std::string version = extension.substr(lastSlashPos + 1);

    // std::cout << "version |" << version << "|\n"; exit (0);

    type = defineMimeType(version);
    headersResponses.insert(std::make_pair(CONTENT_TYPE, type));

}


responseBuilder& responseBuilder::addStatusLine(const std::string &type) {
    defineStatusLine(type);
    return (*this);
}

responseBuilder& responseBuilder::addContentType(const std::string &extension) {
    // std::cout << "HA L3AAAAR|" << extension << "|\n";
    // exit (0);
    //if contenr type is not defined, define it
    if (headersResponses.find(CONTENT_TYPE) == headersResponses.end())
        defineContentType(extension);
    return (*this);
}

responseBuilder& responseBuilder::addLocation(std::string location) {

    location += "/";
    headersResponses.insert(std::make_pair(LOCATION, location));
    return (*this);
}

responseBuilder& responseBuilder::addContentLength() {

    size_t length = body.length();
    std::stringstream ss;

    ss << length;
    if (headersResponses.find(CONTENT_LENGTH) == headersResponses.end())
        headersResponses.insert(std::make_pair(CONTENT_LENGTH, ss.str()));
    return (*this);
}


responseBuilder& responseBuilder::addCookie(const std::string &cookie) {

    headersResponses.insert(std::make_pair("Set-Cookie:", cookie));
    return (*this);
}

responseBuilder& responseBuilder::addLocationFile(const std::string &location) {

    headersResponses.insert(std::make_pair("Location:", location));
    return (*this);
}

responseBuilder& responseBuilder::addContentLength(const std::string &content) {

   unsigned long number = content.size();
   std::ostringstream oss ;

    oss << number ;
    if (headersResponses.find(CONTENT_LENGTH) == headersResponses.end())
        headersResponses.insert(std::make_pair(CONTENT_LENGTH, oss.str()));
    return (*this);
}

responseBuilder& responseBuilder::addResponseBody(const std::string &responseBody) {

    if (this->body.length() == 0) {
        this->body = responseBody;
        this->addContentLength();
    }
    return (*this);

}

std::string responseBuilder::build() {

    std::stringstream response;

    response << HTTP_VERSION << " " << resultMsg << CRLF;

    for (std::multimap<std::string, std::string>::iterator it = headersResponses.begin(); it !=  headersResponses.end(); it++) {
        response << it->first << it->second << CRLF;
    }
    response << "Keep-Alive: timeout=5" << CRLF;
    response << CRLF;

    if (body.length() != 0) {
        response << body << CRLF;
    }
    return response.str();
}
