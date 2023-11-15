#include "webserve.hpp"
#include <sys/types.h>
#include <dirent.h>

static bool characterNotAllowed(std::string &uri) {

    std::string allowedCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

    for (size_t i = 0; i < uri.length(); i++) {
        if (allowedCharacters.find(uri[i]) == std::string::npos )
            return true;
    }
    return false;
}

std::vector<std::string> splitUri(const std::string& input, const std::string& delimiter) {

    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = 0;

    while ((end = input.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(input.substr(start, end - start + delimiter.length()));
        start = end + delimiter.length();
    }
    tokens.push_back(input.substr(start));

    return tokens;
}

static std::string fetchTheExactDirectory(const std::string uri) {

    //? Example: /root/etc/bin/index.html -> location_match_directory = /root/etc/bin/
    //? Example: /Desktop -> /Desktop

    DIR *dir_ptr; dir_ptr = opendir(uri.c_str());
    if (dir_ptr != NULL) {
        if (closedir(dir_ptr) == -1) {
        std::cout << "Error: cannot close the directory" << std::endl; 
        throw "Error: closedir()"; }
        return (uri);
    }  

    //! REASON 
    // if (closedir(dir_ptr) == -1) {
    //     std::cout << "Error: cannot close the directory" << std::endl; 
    //     throw "Error: closedir()";
    // }

    std::vector<std::string> vectorDirectory = splitUri(uri, "/");
    if ( vectorDirectory.size() == 1 ) {
        return (uri);
    }

    std::string concatenateDirectories;
    for (const_vector_it it = vectorDirectory.begin(); it != vectorDirectory.end(); ++it) {
        std::string directory = (*it);
        if (directory[directory.length() - 1] == '/')
            concatenateDirectories += directory;
    }

    concatenateDirectories.erase(concatenateDirectories.end() - 1);
    return (concatenateDirectories);
}

static void removeLastOccurrence(std::string &str) {

    size_t pos = str.rfind('/');

    if (pos != std::string::npos) {
        str.erase(pos, str.length() - 1);
    }

}

//! CAUTION
static void removeExtraBackslashes(std::string& str) {
    std::string result;
    bool previousIsForwardSlash = false;

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '/' && previousIsForwardSlash) {
            // Skip consecutive forward slashes
            continue;
        }

        result += str[i];
        previousIsForwardSlash = (str[i] == '/');
    }

    // Update the original string
    str = result;
}

static std::map<std::string, std::string> fetchSuitableLocationBlock(Request &request, std::string uri) 
{
    //! erase all the backslashes at the end of URI
    //TODO : /////////////////
    if ( ! (uri.length() == 1) ) {
        while (1) {
            if (uri[uri.length() - 1] == '/')
            { 
                request.setSaveLastBS(true);
                uri.erase(uri.length() - 1);
            }
            else
                break ;
        }
    } else {
        request.setSaveLastBS(true);
    }

    //! erase the double slash and count how many for saving the uri if it's only /regular
    int backSlashcount = 0;
    // for (size_t i = 0; i < uri.size(); i++) {
    //     if (uri[i] == '/' && uri [i + 1] && uri [i + 1] == '/') {
    //         uri.erase(uri[i]);
    //         backSlashcount++;
    //     } else if (uri[i] == '/') {
    //         backSlashcount++;
    //     }
    // }

    if (uri.length() != 1) {
        removeExtraBackslashes(uri);
    }

    int countDown = uri.length();

    if (countDown != 1) {
        while (countDown--) {
            if (uri[countDown] == '/')
                backSlashcount++;
        }

    }

    //TODO: RESET URI 
    request.setUri(uri);

    std::vector<std::map<std::string, std::string> > locationsBlock = request.getLocationsBlock();
    std::map<std::string, std::string> found ;

    // if (backSlashcount == 1) {


        for (vectorToMapIterator it = locationsBlock.begin(); it != locationsBlock.end(); ++it) {

            std::map<std::string, std::string> mapIterator = (*it);
            if (mapIterator["location match"] == uri) {
                found = mapIterator ;
            } 
            // else if (mapIterator["location match"] == "/") {
            //     defaultLocation = mapIterator ;
            // }
        }

        if ( ! found.empty() )
            return (found);
        //return (defaultLocation);

    // }




    std::string directoryUri = fetchTheExactDirectory(uri);


    for (vectorToMapIterator it = locationsBlock.begin(); it != locationsBlock.end(); ++it) {

        std::map<std::string, std::string> mapIterator = (*it);
        std::string location_match = mapIterator["location match"];

        if ( location_match == directoryUri )
            found = (mapIterator); break ;

    }

    if ( ! found.empty() )
        return (found);

    std::string substrUri = uri; unsigned long i = 0;

    while (i++ < locationsBlock.size()) {

        removeLastOccurrence(substrUri);

        for (vectorToMapIterator it = locationsBlock.begin(); it != locationsBlock.end(); ++it) {

            std::map<std::string, std::string> mapIterator = (*it);
            std::string location_match = mapIterator["location match"];

            if ( location_match == substrUri ) {
                found = (mapIterator) ;
                goto outerLoop;
            }
        }
    }
    outerLoop: 

    return (found);
}

void validateRequest(Request &request) {

    std::string response;
    std::map<std::string, std::string> httpRequestHeaders = request.getHttpRequestHeaders();

    std::string transferEncoding = httpRequestHeaders["Transfer-Encoding:"];
    if ( ! transferEncoding.empty() && transferEncoding != "chunked") {
        response = "HTTP/1.1 501 Not Implemented\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 41\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1>501 Not Implemented</h1></html>\r\n"; request.setResponseVector(response);
        throw "501" ;
    }

    std::string contentLenghStr = (httpRequestHeaders["Content-Length:"]); int contentLength = std::atoi(contentLenghStr.c_str());
    std::string method = request.getHttpVerb();
    if (method == "POST" && contentLength == 0 && transferEncoding.empty()) {
        response = "HTTP/1.1 400 Bad Request\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 37\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1>400 Bad Request</h1></html>\r\n"; request.setResponseVector(response);
        throw "400" ;        
    }

    std::string uri = request.getUri();
    if ( ! uri.empty() && characterNotAllowed( uri ) ) {
        response = "HTTP/1.1 400 Bad Request\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 37\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1>400 Bad Request</h1></html>\r\n"; request.setResponseVector(response);
        throw "414" ;
    }
    if ( ! uri.length()  &&  uri.length() > 2048) {
        response = "HTTP/1.1 414 Request-URI Too Long\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 46\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1>414 Request-URI Too Long</h1></html>\r\n"; request.setResponseVector(response);
        throw "414" ;
    }     

    //request.setAllowRequestBodyChunk(true);
    request.setRequestBodyChunk(true);
    std::map<std::string, std::string> directives = request.getDirectives();

    unsigned long clientMaxBody = std::atoi((directives["client_max_body_size"]).c_str());

    if (request.getRequestBodyChunk() == true) {
        if ( clientMaxBody && (request.getRequestBody()).length() > clientMaxBody ) {
            response = "HTTP/1.1 413 Request Entity Too Large\r\n"; request.setResponseVector(response);
            response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
            response = "Content-Length: 51\r\n\r\n"; request.setResponseVector(response);
            response = "<html><h1>413 Request Entity Too Large</h1></html>\r\n"; request.setResponseVector(response);
            throw "413";
        }
    }

    //! Skipped: if => no location match the request uri
    std::map<std::string, std::string> location = fetchSuitableLocationBlock(request, uri);

    // for (auto it : location) {
    //     std::cout << "|" << it.first << "|\t|" << it.second << "|\n";
    // }
    // exit (0);

    // std::string root;

    // mapConstIterator it = request.getDirectives().find("root");

    // std::string locationNotFound;
    // root = it->second;
    // locationNotFound = root + request.getUri();

    if ( ! location.empty() ) {
        request.setLocationBlockWillBeUsed(location);
    
    } else if (  request.getLocationBlockWillBeUsed().empty() ) {

        std::map<std::string, std::string> defaultLocation = request.getDirectives();
        request.setLocationBlockWillBeUsed(defaultLocation);

    } else {

        response = "HTTP/1.1 404 Not Found\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 36\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1> 404 Not Found</h1></html>\r\n"; request.setResponseVector(response);
        throw "4042";  

    }

    if ( location.find("allowedMethods") != location.end()) {
        if (location["allowedMethods"] != request.getHttpVerb()) {
            response = "HTTP/1.1 405 Method Not Allowed\r\n"; request.setResponseVector(response);
            response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
            response = "Content-Length: 44\r\n\r\n"; request.setResponseVector(response);
            response = "<html><h1>405 Method Not Allowed</h1></html>\r\n"; request.setResponseVector(response);
            throw "405";            
        }
    }

    if ( location.find("return") != location.end()) {
        response = "HTTP/1.1 301 Moved Permanently\r\n"; request.setResponseVector(response);
        response = "Location: " + location["return"] + "/\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 43\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1>301 Moved Permanently</h1></html>\r\n"; request.setResponseVector(response);
        throw "301";
    }


}
