#include "../includes/webserve.hpp"

void checkRequestedHttpMethod(Request &request) {
    // std::cout << "HTTP Method: " << request.getHttpVerb() << std::endl;
    std::string httpMethod = request.getHttpVerb();
    if (httpMethod == "GET")
        method(request, &getFile, &getFolder);
    else if (httpMethod == "POST")
        method(request, &postFile, &postFolder);
    else
        method(request, &deleteFile, &deleteFolder);
}
