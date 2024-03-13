#include "../includes/webserve.hpp"

void requestTypeDirectoryPost(std::string &root, std::string &uri, Request &request) {

    if ( !request.getSaveLastBS() ) {
        request.response = responseBuilder()
            .addStatusLine("301")
            .addContentType("text/html")
            .addLocation(uri)
            .addResponseBody("<html><h1>301 Moved Permanently</h1></html>");
        throw "301";
    }

    std::map<std::string, std::string> directives = request.getLocationBlockWillBeUsed();
    mapConstIterator it = directives.find("index");

    //* Index file if it exists
    if (it != directives.end()) {
        std::string indexFile = it->second;

        // Remove extra "/" from start of root
        for (size_t i = 0; i < root.length(); i++) {
            if (root[i] == '/' && root[i+1] && root[i+1] == '/') {
                root = root.erase(i, 1);
            }
        }

        // If index file contains the root directory in it remove it
        if (indexFile.find(root) != std::string::npos) {
            indexFile = indexFile.substr(root.length());
        }

        std::string absolutePath = "";
        std::vector<std::string> splitedPaths;

        // Insert all index files splitted by space into a vector
        std::istringstream iss(indexFile);
        std::string token;
        while (std::getline(iss, token, ' ')) {
            splitedPaths.push_back(token);
        }

        // Decide which index file to use
        for (size_t i = 0; i < splitedPaths.size(); i++) {
            std::fstream file((root + '/' + splitedPaths[i]).c_str());
            if ( file.good() ) {
                absolutePath = CheckPathForSecurity(root+'/'+splitedPaths[i]);
                break;
            }
        }

        if (!absolutePath.empty()) {

            std::pair<std::string, std::string> response;
            std::string extension = absolutePath.substr(absolutePath.find_last_of('.'));

            std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
            std::string binaryPath;

            if (isValidCGI(locationBlock, extension, binaryPath)) {

                std::cout << "╔══════════════════════╗\n";
                std::cout << "║         CGI          ║\n";
                std::cout << "╚══════════════════════╝\n";

                response = handleCgiPost(absolutePath, binaryPath, request);
                std::string headers = response.first;
                std::string body = response.second;

                std::string contentType = extractContentType(headers);
                std::string contentLength = ftToString(body.length());

                std::size_t lastSlashPos = contentType.rfind('/');
                std::string type = (lastSlashPos != std::string::npos) ? contentType.substr(lastSlashPos + 1) : "txt";

                std::multimap<std::string, std::string> splitedHeaders = parseResponseHeaders(headers);

                // Set the initial HTTP response headers
                request.response = responseBuilder()
                    .addStatusLine("200")
                    .addContentType(type);
                for (std::multimap<std::string, std::string>::iterator it = splitedHeaders.begin(); it != splitedHeaders.end(); it++) {
                    if (it->first == "Set-Cookie")
                        request.response.addCookie(it->second);
                    else if (it->first == "Location")
                        request.response.addLocationFile(it->second);
                    else if (it->first == "Status") {
                        std::string status = it->second;
                        std::stringstream ss(status);
                        std::string statusNumber;
                        ss >> statusNumber;

                        request.response.addStatusLine(statusNumber);
                    }
                }
                request.response.addResponseBody(body);

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

void requestTypeFilePost(std::string &absolutePath, std::string &uri, Request &request) {

    std::pair<std::string, std::string> response;
    size_t pos = uri.rfind('/');
    std::string file = uri.erase(0, pos);

    if ( file.find('.') != std::string::npos ) {

        std::string extension = file.substr(file.find_last_of('.'));
        std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
        std::string binaryPath;

        if (isValidCGI(locationBlock, extension, binaryPath)) {

            std::cout << "╔══════════════════════╗\n";
            std::cout << "║         CGI          ║\n";
            std::cout << "╚══════════════════════╝\n";

            response = handleCgiPost(absolutePath, binaryPath, request);
            std::string headers = response.first;
            std::string body = response.second;

            std::string contentType = extractContentType(headers);
            std::string contentLength = ftToString(body.length());

            std::size_t lastSlashPos = contentType.rfind('/');
            std::string type = (lastSlashPos != std::string::npos) ? contentType.substr(lastSlashPos + 1) : "txt";

            std::multimap<std::string, std::string> splitedHeaders = parseResponseHeaders(headers);

            // Set the initial HTTP response headers
            request.response = responseBuilder()
                .addStatusLine("200")
                .addContentType(type);
            for (std::multimap<std::string, std::string>::iterator it = splitedHeaders.begin(); it != splitedHeaders.end(); it++) {
                if (it->first == "Set-Cookie")
                    request.response.addCookie(it->second);
                if (it->first == "Location")
                    request.response.addLocationFile(it->second);
                else if (it->first == "Status") {
                    std::string status = it->second;
                    std::stringstream ss(status);
                    std::string statusNumber;
                    ss >> statusNumber;

                    request.response.addStatusLine(statusNumber);
                }
            }
            request.response.addResponseBody(body);

            throw ("CGI");
        }
    }

    request.response = responseBuilder()
        .addStatusLine("403")
        .addContentType("text/html")
        .addResponseBody(request.getPageStatus(403));
    throw "403";

}

void postMethod(Request &request) {

    std::cout << "╔═══════════════════════════╗\n";
    std::cout << "║        POST Method        ║";std::cout << "\tURI: " << request.getUri() << "\n";
    std::cout << "╚═══════════════════════════╝\n";

    uploadRequestBody(request);

    std::map<std::string, std::string> locations = request.getLocationBlockWillBeUsed();

    std::map<std::string, std::string>::const_iterator itContentType = (request.getHttpRequestHeaders()).find("Content-Type");
    std::string value = "";
    if (itContentType != (request.getHttpRequestHeaders()).end()) {
        value = itContentType->second;
    }

    if ( value == "multipart/form-data;" ) {
        multipartContentType(request);
        request.response = responseBuilder()
            .addStatusLine("201")
            .addContentType("text/html")
            .addLocationFile(locations["upload_store"])
            .addResponseBody(request.getPageStatus(201));
        throw "201" ;
    }


    //* get_requested_resource()
    std::string root;
    retrieveRootAndUri(request, root);

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
            // root =  request.getDirectives().find("root")->second;

    // }

    std::string uri = request.getUri();
    if (uri.find('?') != std::string::npos) {
        parseQueriesInURI(request, uri);
    }

    uri = decodeUrl(uri);
    request.setUri(uri);

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

    //TODO: fix this error http://localhost:1111/../../bin/ls the response don't get send

    const char *path = absolutePath.c_str();
    struct stat fileStat;

    if (stat(path, &fileStat ) == 0) {

        if (S_ISREG(fileStat.st_mode)) {
            requestTypeFilePost(absolutePath, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            requestTypeDirectoryPost(absolutePath, uri, request);
        } else {
            request.response = responseBuilder()
                .addStatusLine("502")
                .addContentType("text/html")
                .addResponseBody("<html><h1>502 Bad Gateway</h1></html>");
            throw "502";
        }
    } else {

        request.response = responseBuilder()
            .addStatusLine("404")
            .addContentType("text/html")
            .addResponseBody("<html><h1> 404 Not Found</h1></html>");
        throw "404 here";
    }

}
