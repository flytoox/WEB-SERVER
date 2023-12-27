#include "webserve.hpp"

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
    //if (type == "501") 
        return NOT_IMPLEMENTED;
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
    }

    std::cout <<"generated |" << ret << "|\n";

    headersResponses.insert(std::make_pair(HTTP_VERSION, ret));
}

void responseBuilder::defineContentType(const std::string &extension) {
    std::string res;
    res += extension == "mp4" ? "video/mp4" : "text/html";

    headersResponses.insert(std::make_pair(CONTENT_TYPE,extension));
}


responseBuilder& responseBuilder::addStatusLine(const std::string &type) {
    defineStatusLine(type);
    return (*this);
}

responseBuilder& responseBuilder::addContentType(const std::string &extension) {
    defineContentType(extension);
    return (*this);
}

responseBuilder& responseBuilder::addLocation(const std::string &location) {

    headersResponses.insert(std::make_pair(LOCATION, location));
    return (*this);
}

responseBuilder& responseBuilder::addContentLength() {

    size_t length = body.length();
    std::stringstream ss;

    ss << length;
    headersResponses.insert(std::make_pair(CONTENT_LENGTH, ss.str()));
    return (*this);
}

responseBuilder& responseBuilder::addResponseBody(const std::string &responseBody) {

    this->body = responseBody;
    this->addContentLength();
    return (*this);

}

std::string responseBuilder::build() {

    std::stringstream response;

    response << HTTP_VERSION;
    for (auto it : headersResponses) {
        response << it.first << it.second << CRLF;
    }

    response << CRLF;

    if (body.length() != 0) {
        response << body << CRLF;
    } 

    return response.str();

}
