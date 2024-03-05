#include "../includes/webserve.hpp"


static void uploadRequestBody(Request &request) {

    std::map<std::string, std::string>::const_iterator itContentType;

    itContentType = (request.getHttpRequestHeaders()).find("Content-Type:");
    if ( itContentType != (request.getHttpRequestHeaders()).end()) {

        std::string value = itContentType->second;

        if (value == "text/plain") {
            std::string ret = request.getRequestBody();
            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType("text/html")
            .addResponseBody(ret);
            throw("textContentType");
            // textContentType(request);
        } else if ( value == "multipart/form-data;" ) {
            // multipartContentType(request);
            // request.response = responseBuilder()
            // .addStatusLine("200")
            // .addContentType("text/html")

            // // request.response = responseBuilder()
            // .addResponseBody("<html><h1> Successfully Uploaded </h1></html>");
            // throw "201" ;
        } else if (value == "application/x-www-form-urlencoded") {
            std::cout << "111\n";

            urlencodedContentType(request);
        } else {
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody(request.getPageStatus(400));
                throw "400 CT";
        }
    }

}

// Function to parse HTTP headers string into a map
std::multimap<std::string, std::string> parseResponseHeaders(const std::string& headers) {
    std::multimap<std::string, std::string> headersMap;
    std::istringstream iss(headers);

    std::string line;
    while (std::getline(iss, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            headersMap.insert(std::make_pair(key, value));
        }
    }

    return headersMap;
}


void requestTypeFilePost(std::string &absolutePath, std::string &uri, Request &request) {

    std::pair<std::string, std::string> response;
    size_t pos = uri.rfind('/');
    std::string file = uri.erase(0, pos);

    if ( file.find('.') != std::string::npos ) {

        std::string extension = file.substr(file.find_last_of('.'));
        std::cout << "EXTENSION: " << extension << "\n";
        std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
        std::string binaryPath;

        if (isValidCGI(locationBlock, extension, binaryPath)) {
            // std::map<std::string, std::string> postData = request.getUrlencodedResponse();
            response = handleCgiPost(absolutePath, binaryPath, request);

            std::string headers = response.first;
            std::string body = response.second;

            std::string contentType = extractContentType(headers);
            // Extract extension from "Content-Type"
            std::size_t lastSlashPos = contentType.rfind('/');
            std::string extension = (lastSlashPos != std::string::npos) ? contentType.substr(lastSlashPos + 1) : "";

            std::string contentLength = std::to_string(body.length());

            std::cout << "contentType: " << contentType << "\n";
            std::cout << "extension: " << extension << "\n";

            std::cout << "HEADERS |" << headers << "|\n";
            std::multimap<std::string, std::string> splitedHeaders = parseResponseHeaders(headers);
            for (std::multimap<std::string, std::string>::iterator it = splitedHeaders.begin(); it != splitedHeaders.end(); it++) {
                std::cout << "HEADER |" << it->first << " : " << it->second << "|\n";
            }
            std::cout << "BODY |" << body << "|\n";

            // Set the initial HTTP response headers
            request.response = responseBuilder()
            .addStatusLine("302")
            .addContentType(extension)
            .addResponseBody(body);
            for (std::multimap<std::string, std::string>::iterator it = splitedHeaders.begin(); it != splitedHeaders.end(); it++) {
                if (it->first == "Set-Cookie")
                    request.response.addCookie(it->second);
                else if (it->first == "Location")
                    request.response.addLocationFile(it->second);
            }
            request.response.addResponseBody(body);

            // request.response.addCookie("loggedIn=true; expires=Wed, 03-Apr-2024 17:37:03 GMT; Max-Age=2592000; path=/");
            throw ("CGI");
        }
    }

    request.response = responseBuilder()
    .addStatusLine("403")
    .addContentType("text/html")
    .addResponseBody(request.getPageStatus(403));
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

    std::pair<std::string, std::string> response;

    //* Index file if it exists
    if (it != directives.end()) {
        std::string indexFile = it->second;

        //remove extra / from root at start of root
        for (size_t i = 0; i < root.length(); i++) {
            if (root[i] == '/' && root[i+1] && root[i+1] == '/') {
                root = root.erase(i, 1);
            }
        }
        //if index file contains the root directory in it remove it
        if (indexFile.find(root) != std::string::npos) {
            indexFile = indexFile.substr(root.length());
        }

        std::string absolutePath = "";
        std::string requestBody = request.getRequestBody();

        std::vector<std::string> splitedPaths;
        //splite absolutePath with whiteSpaces
        std::istringstream iss(indexFile);
        std::string token;
        while (std::getline(iss, token, ' ')) {
            splitedPaths.push_back(token);
        }

        for (size_t i = 0; i < splitedPaths.size(); i++) {
            std::fstream file(root+'/'+splitedPaths[i]);
            if ( file.good() ) {
                absolutePath = CheckPathForSecurity(root+'/'+splitedPaths[i]);
                break;
            }
        }

        if (!absolutePath.empty()) {
            std::string extension = absolutePath.substr(absolutePath.find_last_of('.'));
            std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
            std::string binaryPath;

            if (isValidCGI(locationBlock, extension, binaryPath)) {
                // std::map<std::string, std::string> postData = request.getUrlencodedResponse();

                response = handleCgiPost(absolutePath, binaryPath, request);
                std::string headers = response.first;
                std::string body = response.second;

                std::string contentType = extractContentType(headers);
                // Extract extension from "Content-Type"
                std::size_t lastSlashPos = contentType.rfind('/');
                std::string extension = (lastSlashPos != std::string::npos) ? contentType.substr(lastSlashPos + 1) : "";

                std::string contentLength = std::to_string(body.length());

                std::cout << "contentType: " << contentType << "\n";
                std::cout << "extension: " << extension << "\n";

                std::cout << "HEADERS |" << headers << "|\n";
                std::multimap<std::string, std::string> splitedHeaders = parseResponseHeaders(headers);
                std::cout << "BODY |" << body << "|\n";

                // Set the initial HTTP response headers
                request.response = responseBuilder()
                    .addStatusLine("200")
                    .addContentType(extension)
                    .addResponseBody(body);
                for (std::multimap<std::string, std::string>::iterator it = splitedHeaders.begin(); it != splitedHeaders.end(); it++) {
                    if (it->first == "Set-Cookie")
                        request.response.addCookie(it->second);
                    else if (it->first == "Location")
                        request.response.addLocationFile(it->second);
                }
                request.response.addContentType(extension).addResponseBody(body);
                throw ("CGI");
            }
        }
    }

    request.response = responseBuilder()
    .addStatusLine("403")
    .addContentType("text/html")
    .addResponseBody(request.getPageStatus(403));
    throw ("403");
}


void parseQueriesInURI(Request &request,std::string &uri) {

   // uri = /?username=sana&password=123

    //* Protect the quesries if they exist

    size_t pos = uri.find('?');
    if (uri.length() == pos + 1)
        return ;

    std::string queriesString = uri.substr(uri.find('?') + 1);
    request.setQueryString(queriesString);
    std::map<std::string, std::string> mapTopush;

    std::stringstream ss(queriesString);
    std::vector<std::string> keyValueVector;
    std::string token;

    while (std::getline(ss, token, '&')) {
        keyValueVector.push_back(token);
    }

    for (auto it : keyValueVector ) {
        std::cout << "RESULT ||" << it << "||\n";
    }

    for (const_vector_it it = keyValueVector.begin(); it != keyValueVector.end(); it++) {
        std::string keyValue = (*it);
        size_t signPos = keyValue.find('=');
        try {
            if (keyValue[0] == '?') {
                throw (std::runtime_error("400"));
            }
            if (signPos != std::string::npos) {
                if (keyValue.substr(signPos + 1).empty())
                    throw (std::runtime_error("400"));
                pair pair = std::make_pair(keyValue.substr(0, signPos), keyValue.substr(signPos + 1));
                mapTopush.insert(pair);
            } else {
                throw (std::runtime_error("400"));
            }
        } catch (std::exception &e) {
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody("<html><body><h1>400 Bad Request123</h1></body></html>");
            throw "400";
        }
    }


    request.setUrlencodedResponse(mapTopush);

    // Remove queries from uri
    uri.erase(uri.find('?'));
}

void postMethod(Request &request) {

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

    std::map<std::string, std::string>::const_iterator itContentType = (request.getHttpRequestHeaders()).find("Content-Type:");
    std::string value = "";
    if (itContentType != (request.getHttpRequestHeaders()).end()) {
        value = itContentType->second;
    }


    if (locations["upload_enable"] == "on" && value == "multipart/form-data;") {
            multipartContentType(request);
            request.response = responseBuilder()
                .addStatusLine("201")
                .addContentType("text/html")
                .addResponseBody("<html><h1>201 Created</h1></html>");
            throw "201" ;
        // uploadRequestBody(request);
    }


    //* get_requested_resource()
    std::string root, locationUsed;
    retrieveRootAndUri(request, root, locationUsed);

    // if ( root.empty() ) {

        // mapConstIterator it = request.getDirectives().find("root");
        // if (it == (request.getDirectives()).end()) {


        //     request.response = responseBuilder()
        //     .addStatusLine("200")
        //     .addContentType("text/html")
        //     .addResponseBody("<html><head><title>Welcome to Our Webserver!</title></head><body><p><em>Thank you for using our webserver.</em></p></body></html>");

        //     throw "No Root: 200";
        // }
        // else
            root =  request.getDirectives().find("root")->second;

    // }

    std::string uri = request.getUri();

    // std::cout << "POST : BEFORE URI|" << uri << "|\n";

    if (uri.find('?') != std::string::npos) {
        parseQueriesInURI(request, uri);
    }

    uri = decodeUrl(uri);
    request.setUri(uri);

    // std::cout << "POST : BEFORE URI|" << uri << "|\n";
    std::string absolutePath;
    std::string result =  CheckPathForSecurity(root+uri);
	if (result.find(root) == std::string::npos) {
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(403));
            throw "403 Security";
	}
    absolutePath = result;
    // std::cout << "HAL3aaaaar|" << absolutePath << "|\n";

    // std::string absolutePath = root + uri;

    //DONE: http://localhost:1111/../../tmp/ll.txt check the uri if it bypasses the root dir
    //TODO: fix this error http://localhost:1111/../../bin/ls the response don't get send
    //DONE: seperate uri with queries /uri?ljsl=lsls&ddo=oo

    const char *path = absolutePath.c_str();
    struct stat fileStat;

    if (stat(path, &fileStat ) == 0) {

        if (S_ISREG(fileStat.st_mode)) {
            std::cout << "FILE\n";
            requestTypeFilePost(absolutePath, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
             std::cout << "DIRECTORY\n";
            requestTypeDirectoryPost(absolutePath, uri, request);
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
