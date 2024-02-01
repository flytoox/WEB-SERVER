#include "webserve.hpp"


void checkRequestedHttpMethod(Request &request) {

    
    std::string httpMethod;
    httpMethod = request.getHttpVerb();

    if (httpMethod == "GET") {
        getMethod(request);
    } 
    else if (httpMethod == "POST") {
        postMethod(request);
    } 
    else {
        deleteMethod(request);
    }


}
