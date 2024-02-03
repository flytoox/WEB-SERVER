#include "includes/webserve.hpp"


static void uploadRequestBody(Request &request) {

    // //! CGI MUST BE RUN
    // bool uploaded = cgiRun(request);
    // if (uploaded) {

    //     request.response = responseBuilder()
    //     .addStatusLine("201")
    //     .addContentType("text/html")
    //     .addResponseBody("");

    //     // std::string response = "HTTP/1.1 201 Created\r\n"; request.setResponseVector(response);
    //     // response = "Content-Length: 0\r\n"; request.setResponseVector(response);
    //     // response = "\r\n"; request.setResponseVector(response);

    // }

    // return ( true );

    std::map<std::string, std::string>::const_iterator itContentType;

    // itContentType = (request.getHttpRequestHeaders()).find("Content-Type:");

    // if ( itContentType != (request.getHttpRequestHeaders()).end() ) {

    //     std::string value = itContentType->second;

    //     if ( value == "multipart/form-data;" ) {
    //         hey();
    //         multipartContentType(request);
    //         request.response = responseBuilder()
    //         .addStatusLine("200")
    //         .addContentType("text/html");

    //         request.response = responseBuilder()
    //         .addResponseBody("<html><h1> Successfully Uploaded </h1></html>");
    //         throw "201" ;
    //     } 
    
    // }

    // std::cout << "[][][][][][][]\n";
    // for (auto it : (request.getHttpRequestHeaders())) {
    //     std::cout << "|" << it.first << "|  |" << it.second << "\n";
    // }
    // std::cout << "[][][][][][][]\n";
    // exit (0);


    itContentType = (request.getHttpRequestHeaders()).find("Content-Type:");
    if ( itContentType != (request.getHttpRequestHeaders()).end()) {

        std::string value = itContentType->second;

        if (value == "text/plain") {
            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType("txt");

            textContentType(request);
        } else if ( value == "multipart/form-data;" ) {
            multipartContentType(request);
            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType("text/html");

            request.response = responseBuilder()
            .addResponseBody("<html><h1> Successfully Uploaded </h1></html>");
            throw "201" ;
        } else if (value == "application/x-www-form-urlencoded") {
            std::cout << "111\n";
            urlencodedContentType(request);
        } else {
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody("<html><h1>400 Bad Request</h1></html>");
                throw "400 CT";
        }
    }

}


void requestTypeFilePost(std::string &absolutePath, std::string &uri, Request &request) {

    (void)absolutePath;
    std::string response;

    std::map<std::string, std::string> directives = request.getDirectives();
    size_t pos = uri.rfind('/');

    std::string file = uri.erase(0, pos);


    std::cout << "FILE NAME |" << file << "|\n";
    if ( file.find('.') != std::string::npos ) {

        std::string extension = file.substr(file.find('.'), ( file.length() - file.find('.')) );


        if ( extension == ".php" || extension == ".py") {
            //! RUN POST CGI !
            // response = "HTTP/1.1 200 OK \r\n"; request.setResponseVector(response);
            // response = "Content-type: text/html; charset=UTF-8\r\n\r\n"; request.setResponseVector(response);

            std::cout << "***INFORMATION YOU NEED****\n";
            std::map<std::string, std::string> toUSE = request.getUrlencodedResponse(); 
            for (auto it : toUSE) {
                std::cout << it.first << "|\t|" << it.second << "|\n";
            }
            std::cout << "***INFORMATION YOU NEED****\n";

            // throw "POST CGI";

            request.response = responseBuilder()
                .addStatusLine("200")
                .addContentType("text/html")
                .addResponseBody("<html><h1>POST CGI</h1></html>");
                throw "POST CGI";

        } 
    }

    request.response = responseBuilder()
    .addStatusLine("403")
    .addContentType("text/html")
    .addResponseBody("<html><h1>403 Forbidden</h1></html>");
    throw "403";

}


void requestTypeDirectoryPost(std::string &root, std::string &uri, Request &request) {

    if ( ! request.getSaveLastBS() ) {
        request.response = responseBuilder()
        .addStatusLine("301")
        .addContentType("text/html") //* COOL
        .addLocation(uri)
        .addResponseBody("<html><h1>301 Moved Permanently</h1></html>");
        throw "301";
    }

    // std::map<std::string, std::string> directives = request.getDirectives();
    std::map<std::string, std::string> directives = request.getLocationBlockWillBeUsed();
    mapConstIterator it = directives.find("index");

    std::string absolutePath = root;
    std::string response;

    //* Index file if it exists 
    if (it != directives.end()) {
        std::string indexFile = it->second;
        std::string extension = indexFile.substr(indexFile.find('.'), ( indexFile.length() - indexFile.find('.')) );
        std::string requestBody = request.getRequestBody();
        if ( extension == ".php" || extension == ".py") {
            //! RUN POST CGI !
            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType("text/html")
            .addResponseBody(requestBody);
            throw "CGI";
        } 

    }

    request.response = responseBuilder()
    .addStatusLine("403")
    .addContentType("text/html")
    .addResponseBody("<html><h1>403 Forbidden</h1></html>");
    throw ("403");
}


void oo() {}
void postMethod(Request &request) {

oo();
    //CHECK: I added this function to check the body type 
    uploadRequestBody(request);

    //std::cout << "GOT HERE\n";
    //* if_location_support_upload()
    // for (vectorToMapIterator it = request.getLocationsBlock().begin(); it != request.getLocationsBlock().end(); ++it) {
    //     std::map<std::string, std::string> locationBlock = (*it);
    //     if ( locationBlock["upload_enable"] == "on" ) {
    //         if  ( uploadRequestBody(request) ) 
    //             throw "201" ;  
    //     } 
    // }

    std::map<std::string, std::string> locations = request.getLocationBlockWillBeUsed();
    if (locations["upload_enable"] == "on") {
        uploadRequestBody(request) ;
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

    // std::cout << "POST : BEFORE URI|" << uri << "|\n";

    if (uri.find('?') != std::string::npos) {
        parseQueriesInURI(request, uri);
    }

    // std::cout << "POST : BEFORE URI|" << uri << "|\n";
    std::string absolutePath;
    std::string result =  CheckPathForSecurity(root+uri);
	if (result.find(root) == std::string::npos) {
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody("<html><h1>403 Forbidden for Security Purposes</h1></html>");
            throw "403 Security"; 
	}
    absolutePath = result;
    // std::cout << "HAL3aaaaar|" << absolutePath << "|\n";

    // std::string absolutePath = root + uri;

    //WORKING: http://localhost:1111/../../tmp/ll.txt check the uri if it bypasses the root dir
    //TODO: fix this error http://localhost:1111/../../bin/ls the response don't get send
    //DONE: seperate uri with queries /uri?ljsl=lsls&ddo=oo

    const char *path = absolutePath.c_str(); 
    struct stat fileStat;

    if (stat(path, &fileStat ) == 0) {
    
        if (S_ISREG(fileStat.st_mode)) {
            std::cout << "FILE\n";
            requestTypeFilePost(root, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
             std::cout << "DIRECTORY\n";
            requestTypeDirectoryPost(root, uri, request);
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