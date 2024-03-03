#include "../includes/webserve.hpp"

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

// static void removeLastOccurrence(std::string &str) {

//     size_t pos = str.rfind('/');
//     std::cout << "i: |" << pos << "|\n";

//     if (pos == 0 && pos != std::string::npos) {
//         str = str[0];
//     } else if ( pos != std::string::npos ) {
//         str.erase(pos, str.length() - 1);
//     }

// }

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
    //* WE remove it for stat function and then we check if it exsits
    //* if yes and it's a directory, then handle it without moving 301 Permenatntly
    //* if no it's a directory , add '/' to move permenantly
    if (uri.length() != 1) {
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

    std::vector<std::map<std::string, std::string>> locationsBlock = request.getLocationsBlock();
    std::map<std::string, std::string> found ;

    for (vectorToMapIterator it = locationsBlock.begin(); it != locationsBlock.end(); ++it) {

        std::map<std::string, std::string> mapIterator = (*it);
        std::string location_match = mapIterator["location"];

        if ( location_match == uri ) {
           return (mapIterator);
        }

    }

    std::cout << "URI |" << uri << "|\n";
    for (vectorToMapIterator it = locationsBlock.begin(); it != locationsBlock.end(); ++it) {

        std::map<std::string, std::string> mapIterator = (*it);
        std::string location_match = mapIterator["location"];

        if ( location_match == uri ) {
           return (mapIterator);
        }

    }

    std::string directoryUri = fetchTheExactDirectory(uri);
    std::cout << "WHAAAAAAAT|" << directoryUri << "|\n";

    while (!directoryUri.empty()) {
        for (auto it = locationsBlock.begin(); it != locationsBlock.end(); it++) {
            if (it->at("location") == directoryUri) {
                std::cerr << "OK OK ?? " << directoryUri<< std::endl;
                return *it;
            }
        }
        int ind = directoryUri.find_last_of('/');
        if (ind != -1)
            directoryUri.erase(ind);
        else directoryUri.clear();
    }
    return (found);
}
void validateRequest(Request &request) {

    std::map<std::string, std::string> httpRequestHeaders = request.getHttpRequestHeaders();


    std::string transferEncoding = httpRequestHeaders["Transfer-Encoding:"];
    if ( !transferEncoding.empty() && transferEncoding != "chunked") {

        request.response = responseBuilder()
            .addStatusLine("501")
            .addContentType("text/html")
            .addResponseBody("<html><h1>501 Not Implemented</h1></html>");
        throw "501" ;

    }

    // std::string contentLenghStr = (httpRequestHeaders["Content-Length:"]);
    // int contentLength = std::atoi(contentLenghStr.c_str());

    //! This was done to check the POST method and Transfer-Encoding and Content-Length
    // mapConstIterator contentLengh = httpRequestHeaders.find("Content-Length:");

    // std::string method = request.getHttpVerb();
    // if (method == "POST" && contentLengh == httpRequestHeaders.end() ) {

    //     request.response = responseBuilder()
    //         .addStatusLine("400")
    //         .addContentType("text/html")
    //         .addResponseBody("<html><h1>400 Bad Request20</h1></html>");

    //     throw "40018" ;
    // }

    std::string uri = request.getUri();
    //! TEST BUILDER PATTERN

    if ( !uri.empty() && characterNotAllowed( uri ) ) {
        request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody("<html><h1>400 Bad Request21</h1></html>");
        throw "414";
    }

    if ( uri.length()  &&  uri.length() > 2048) {

        request.response = responseBuilder()
        .addStatusLine("414")
        .addContentType("text/html")
        .addResponseBody("<html><h1>414 Request-URI Too Long</h1></html>");

        throw "414" ;
    }

    // request.setRequestBodyChunk(true);
    //request.setAllowRequestBodyChunk(true);
    std::map<std::string, std::string> directives = request.getDirectives();


    unsigned long clientMaxBody = std::atoi((directives["client_max_body_size"]).c_str());

    if (request.getRequestBodyChunk() == true) {
        if ( clientMaxBody && (request.getRequestBody()).length() > clientMaxBody ) {

            request.response = responseBuilder()
            .addStatusLine("413")
            .addContentType("text/html")
            .addResponseBody("<html><h1>413 Request Entity Too Large</h1></html>");

            throw "413";
        }
    }

    //! Skipped: if => no location match the request uri
    std::map<std::string, std::string> location = fetchSuitableLocationBlock(request, uri);
    // std::string root;

    // mapConstIterator it = request.getDirectives().find("root");

    // std::string locationNotFound;
    // root = it->second;
    // locationNotFound = root + request.getUri();

    if ( ! location.empty() ) {
        request.setLocationBlockWillBeUsed(location) ;
    } else if ( request.getLocationBlockWillBeUsed().empty() ) {

        // std::cout << "GET HERE\n";

        std::map<std::string, std::string> defaultLocation = request.getDirectives();

        // for (auto it : defaultLocation) {
        //     std::cout << "|" << it.first << "|\t|" << it.second << "|\n";
        // }
        request.setLocationBlockWillBeUsed(defaultLocation);
        location = request.getLocationBlockWillBeUsed();

    } else {

        request.response = responseBuilder()
        .addStatusLine("404")
        .addContentType("text/html")
        .addResponseBody("<html><h1> 404 Not Found</h1></html>");
        throw "4042";
    }


    std::map<std::string, std::string> why = request.getLocationBlockWillBeUsed();

    // std::cout << "*************TESTING***********************\n";
    // std::cout << " --------> URI |" << request.getUri() << "| <---------------\n";

    // for (auto it : why) {
    //     std::cout << "|" << it.first << "|\t|" << it.second << "|\n";
    // }
    // std::cout << "*************TESTING***********************\n";
    // exit (0);


    if ( location.find("return") != location.end() ) {

     std::string changeLocation = location["return"];
    std::vector<std::string> vectorReturn = splitString(changeLocation, " ");
    changeLocation = *(vectorReturn.end() - 1);
        std::cout << "CORRECT ? |" << changeLocation << "|\n";
        //TODO:this must be split; the first string must contain 301 and the second strign is the one saved as a value for directives["return"]

        if ( changeLocation.length() ) {
            if (changeLocation.substr(0, 8) != "https://") {
                changeLocation.insert(0, "https://");
            }
            request.response = responseBuilder()
            .addStatusLine(*vectorReturn.begin())
            .addLocation(changeLocation)
            .addContentType("text/html")
            .addResponseBody("<html><h1>" + (*vectorReturn.begin()) + "</h1></html>");

            throw "return directive";
        }
        // else {

        //     request.response = responseBuilder()
        //     .addStatusLine("200")
        //     .addContentType("text/html")
        //     .addResponseBody("<html><h1>301 Moved Permanently</h1></html>");
        //     throw "200";
        // }

        std::map<std::string, std::string> directives = request.getDirectives();
        std::string returnCheck = directives["return"];
        std::vector<std::map<std::string, std::string> > allLocations = request.getLocationsBlock();
        for (vectorToMapIterator it = allLocations.begin(); it != allLocations.end(); it++) {
            std::map<std::string, std::string> locations = *it;
            if ( locations["location"] == returnCheck  ) {
                if ( directives["location"] != returnCheck )
                    request.setUri(returnCheck);
                    request.setLocationBlockWillBeUsed(locations);
                    break ;
            }
        }

    }

    //DONE: this is a mqp which containes notAllowd methods ; must be split and checked !
    //DONE : Omar this a string not a word to override everytime
    if ( location.find("allowedMethods") != location.end()) {

        std::string input = location["allowedMethods"];
        std::cout << "Allowed Methods |" << input << "|\n";
        std::vector<std::string> theAllowedMethods = splitString(input, " ");
        for (auto it : theAllowedMethods) {
            std::cout << "|" << it << "|\n";
        }
        const_vector_it itAllowedMethods = std::find(theAllowedMethods.begin(), theAllowedMethods.end(), request.getHttpVerb());


        if (itAllowedMethods == theAllowedMethods.end()) {

            request.response = responseBuilder()
            .addStatusLine("405")
            .addContentType("text/html")
            .addResponseBody("<html><h1>405 Method Not Allowed</h1></html>");

            throw "405";
        }
    }

}
