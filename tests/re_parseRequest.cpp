#include "webserve.hpp"

static bool characterNotAllowed(std::string &uri) {

    std::string allowedCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

    for (size_t i = 0; i < uri.size(); i++) {
        if (allowedCharacters.find(i) == std::string::npos )
            return true;
    }
    return false;
}

std::string &fetchLocation(std::string &uri, Request &request) {

    std::string location;


    


    return (location);

}


void checkRequestFormat(Request &request, std::string &requestOutput) {

    //* need to print this value to set it exectly

    std::string transferEncoding = request.getTransferEncoding();
    if ( ! transferEncoding.empty() && transferEncoding != "chunked") {
        requestOutput = "501 Not implemented\n";
        return ;
    }

    std::string contentLength = request.getContentLength();
    std::string method = request.getHttpVerb();
    if ( method == "POST" && transferEncoding.empty() && contentLength.empty() ) {
        requestOutput = "8/ 400 Bad Request\n";
        return ;
    }

    std::string uri = request.getUri();
    if ( characterNotAllowed( uri ) ) {
        requestOutput = "9/ 400 Bad Request\n";
        return ;
    }

    if (uri.length() >= 2048) {
        requestOutput = "414 Request-URI Too Long\n";
        return ;
    }


    //Skipped "413 Request Entity too Large

    std::string defineLocation = fetchLocation(uri, request);

    //Skipped fetchLocation(); "404 Not Found"



}
