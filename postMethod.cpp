#include "webserve.hpp"

static bool cgiRun(Request &request) {

    (void)request ;
    return (false);
}

static bool uploadRequestBody(Request &request) {

    //! CGI MUST BE RUN
    bool uploaded = cgiRun(request);
    if (uploaded) {

        request.response = responseBuilder()
        .addStatusLine("201")
        .addContentType("text/html")
        .addResponseBody("");

        // std::string response = "HTTP/1.1 201 Created\r\n"; request.setResponseVector(response);
        // response = "Content-Length: 0\r\n"; request.setResponseVector(response);
        // response = "\r\n"; request.setResponseVector(response);

    }

    return ( true );

}

void postMethod(Request &request) {


    //std::cout << "GOT HERE\n";
    //* if_location_support_upload()
    for (vectorToMapIterator it = request.getLocationsBlock().begin() ; it != request.getLocationsBlock().end(); ++it) {
        std::map<std::string, std::string> locationBlock = (*it);
        if ( locationBlock["location match"] == "/upload" ) {
            if  ( uploadRequestBody(request) ) 
                throw "201" ;  
        }
    }

    //* get_requested_resource()
    std::string root, locationUsed;
    retrieveRootAndUri(request, root, locationUsed);

    if ( root.empty() ) {

        mapConstIterator it = request.getDirectives().find("root");
        if (it == (request.getDirectives()).end()) {


            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType("text/html")
            .addResponseBody("<html><head><title>Welcome to Our Webserver!</title></head><body><p><em>Thank you for using our webserver.</em></p></body></html>");

            throw "No Root: 200";
        }
        else
            root = it->second;

    }

    std::string uri = request.getUri();
    std::string absolutePath = root + uri;

    const char *path = root.c_str(); struct stat fileStat;

    if (stat(path, &fileStat ) == 0) {
    
        if (S_ISREG(fileStat.st_mode)) {
            requestTypeFile(root, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            requestTypeDirectory(root, uri, request);
        } else {
            request.response = responseBuilder()
            .addStatusLine("502")
            .addContentType("text/html")
            .addResponseBody("<html><h1>502 Bad Gateway</h1></html>");
        }
    } else {
        
        request.response = responseBuilder()
        .addStatusLine("404")
        .addContentType("text/html")
        .addResponseBody("<html><h1> 404 Not Found</h1></html>");
        
        throw "404 here";
    }

}