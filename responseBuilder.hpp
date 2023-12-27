#pragma once

#include <string>

enum STATUS_CODE_ENUM {
    OK ,                        //* 200
    CREATED ,                   //* 201
    NO_CONTENT ,                //* 204
    MOVED_PERMANETLY ,          //* 301
    BAD_REQUEST ,               //* 400
    FORBIDDEN ,                 //* 403
    NOT_FOUND ,                 //* 404
    CONFLICT ,                  //* 409
    REQUEST_TOO_LARGE ,         //* 413
    URI_TOO_LONG ,              //* 414
    INTERNAL_SERVER_ERROR ,     //* 500
    NOT_IMPLEMENTED,            //* 501
    METHOD_NOT_ALLOWED         //*405
};

class responseBuilder {

public:
    
    std::map<std::string, std::string> headersResponses;
    std::string body;

    void defineStatusLine(const std::string &type);
    void defineContentType(const std::string &extension);

    responseBuilder& addStatusLine(const std::string &type);
    responseBuilder& addLocation(const std::string &location);
    responseBuilder& addContentType(const std::string &extension);
    responseBuilder& addContentLength();
    responseBuilder& addResponseBody(const std::string &responseBody);



    std::string build();

};
