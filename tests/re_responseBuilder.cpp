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
    if (type == "501") 
        return NOT_IMPLEMENTED;
}

std::string defineStatusLine(const std::string &type) {

    std::string ret = HTTP_VERSION;


    switch (defineStatusCode(type)) {
        case 0 : ret += RESPONSE_OK; ret += CRLF ; break ;
        case 1 : ret += RESPONSE_CREATED; ret += CRLF ; break ;
        case 2 : ret += RESPONSE_NO_CONTENT; ret += CRLF ; break ;
        case 3 : ret += RESPONSE_MOVED_PERMANETLY; ret += CRLF ; break ;
        case 4 : ret += RESPONSE_BAD_REQUEST; ret += CRLF ; break ;
        case 5 : ret += RESPONSE_FORBIDDEN; ret += CRLF ; break ;
        case 6 : ret += RESPONSE_NOT_FOUND; ret += CRLF; break ;
        case 7 : ret += RESPONSE_METHOD_NOT_ALLOWED; ret += CRLF; break ;
        case 8 : ret += RESPONSE_CONFLICT; ret += CRLF; break ;
        case 9 : ret += RESPONSE_REQUEST_TOO_LARGE; ret += CRLF; break ;
        case 10 : ret += RESPONSE_URI_TOO_LONG; ret += CRLF; break ;
        case 11 : ret += RESPONSE_INTERNAL_SERVER_ERROR; ret += CRLF; break ;
        case 12 : ret += RESPONSE_NOT_IMPLEMENTED; ret += CRLF; break ;
    }

    return (ret);
}

std::string defineContentType(const std::string &extension) {
    std::string res = CONTENT_TYPE;
    res += extension == "mp4" ? "video/mp4" : "text/html";
    res += CRLF ;
    return (res);
}


std::string defineContentLength(const std::string &length) {

    std::string res = CONTENT_LENGTH;

    res += length;
    res += CRLF;
    return (res);

}
//* keep chaining the function calls to build the actual response 
responseBuilder& responseBuilder::addStatusLine(const std::string &type) {
    response = defineStatusLine(type);
    return (*this);
}

responseBuilder& responseBuilder::addLocation(const std::string &location) {
    response = LOCATION; 
    response += location; response += CRLF;
    return (*this);
}

responseBuilder& responseBuilder::addContentType(const std::string &extension) {
    response = defineContentType(extension);
    return (*this);
}

responseBuilder& responseBuilder::addContentLength(const std::string &responseBody) {
    response = defineContentLength(responseBody);
    response += CRLF;
    return (*this);
}

responseBuilder& responseBuilder::addResponseBody(const std::string &responseBody) {
    response = responseBody;
    response += CRLF;
    return (*this);
}    

std::string responseBuilder::build() {
    
    std::stringstream response;
    
    response << "HTTP/1.1 " << resultCode << " " << resultMessage << CRLF;
    
    for (auto it : headers) {
        response << it->first << it->second << CRLF;
    }
    
    if (body.length() > 0) {
        response << "Content-Length: " << body.length() << CRLF;
    }
    
    response << CRLF;
    
    response << body;
    response << CRLF;
    return response.str();
}
 
//* in each add builder function I need to add the header in key map and the value in value map 
//* then in the build function , I  will gather all the strings in that map to generate the response
//* because here I can count the length of the response

//* this builder method won't be used for videos or images m preferably, you read and pass directly to the client as you were doing previously 
//* Need to add some more MIME types 