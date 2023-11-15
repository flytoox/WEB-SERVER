#include "webserve.hpp"

static bool characterNotAllowed(std::string &uri) {

    std::string allowedCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

    for (size_t i = 0; i < uri.length(); i++) {
        if (allowedCharacters.find(i) != std::string::npos )
            return true;
    }
    return false;
}

static bool fetchLocationsAndCompare(Request &request, std::vector<std::map<std::string, std::string> > &locations) {

    std::string requestedUri = request.getUri();

    for (std::vector<std::map<std::string, std::string> >::iterator it = locations.begin(); it != locations.end(); ++it) {
        std::map<std::string, std::string> eachLocation = (*it);
        std::map<std::string, std::string>::iterator found = eachLocation.find(requestedUri);
        if ( found != eachLocation.end() ) {

            for (std::map<std::string, std::string>::const_iterator it = eachLocation.begin(); it != eachLocation.end(); ++it) {
                std::cout << "Key: " << (*it).first << "\t Value: " << (*it).second << std::endl; 
            }

            request.setLocationBlockWillBeUsed(eachLocation);

            break ;
        }
    }

    // std::map<std::string, std::string> location = request.getLocationBlockWillBeUsed();

    // for (std::map<std::string, std::string>::iterator it = location.begin(); it != location.end(); ++it) {
    //     std::cout << "Key: " << (*it).first << "\t Value: " << (*it).second << std::endl; 
    // }


    if ( ! locations.empty() ) {
        return (true);
    }
    return (false);

}


void validateTheRequestFormat(Request &request) {

    std::string responseTest;
    std::vector<std::map<std::string, std::string> > locations;


    std::string transferEncoding = request.getTransferEncoding();
    //std::cout << "Transfer-Encoding: " << transferEncoding << std::endl;
    //std::cout << "TR: " << transferEncoding << std::endl;
    if ( ! transferEncoding.empty() && transferEncoding != "chunked") {
        responseTest = "501 Not implemented\n";
        request.setrequestOutputTest(responseTest);
        return ;
    }


    int contentLength = request.getContentLength();
    //std::cout << "Content-Length: " << contentLength << std::endl;
    //std::cout << "contentLength: " << contentLength << std::endl;
    std::string method = request.getHttpVerb();
    if ( method == "POST" && transferEncoding.empty() && contentLength == 0 ) {
        responseTest = "10/ 400 Bad Request\n";
        request.setrequestOutputTest(responseTest);
        return ;
    }

    std::string uri = request.getUri();
    //std::cout << "uri: " << uri << std::endl;
    if ( ! uri.empty() && characterNotAllowed( uri ) ) {
        responseTest = "11/ 400 Bad Request\n";
        request.setrequestOutputTest(responseTest);
        return ;
    }

    if ( ! uri.length()  &&  uri.length() >= 2048) {
        responseTest = "414 Request-URI Too Long\n";
        request.setrequestOutputTest(responseTest);
        return ;
    }    

    //Skipped "413 Request Entity too Large

    locations = request.getLocationsBlock();
    
    if ( !fetchLocationsAndCompare(request, locations) ) {
        responseTest = "404 Not Found\n";
        return ;
    }

    

    responseTest = "12/ 200 OK Success\n";
    request.setrequestOutputTest(responseTest);
    return ;
    

    //Skipped location_have_redirection "301 Moved Permanently"
    //Skipped method_not_allowd "405 Method Not Allowed"

}
