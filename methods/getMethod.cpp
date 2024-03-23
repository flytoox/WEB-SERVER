#include "../includes/webserve.hpp"

void getFolder(std::string &root, std::string &uri, Request &request) {

    if ( !request.getSaveLastBS() ) {
        request.response = responseBuilder()
            .addStatusLine("301")
            .addContentType("text/html")
            .addLocation(uri)
            .addResponseBody(request.getPageStatus(301));
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
            // std::fstream file(root+'/'+splitedPaths[i]);
            std::fstream file((root + '/' + splitedPaths[i]).c_str());
            if ( file.good() ) {
                absolutePath = CheckPathForSecurity(root+'/'+splitedPaths[i]);
                break;
            }
        }

        if (!absolutePath.empty()) {

            std::pair<std::string, std::string> response;
            std::string extension = absolutePath.substr(absolutePath.find_last_of('.'));

            if (extension != ".html") {

                std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
                std::string binaryPath;

                if (isValidCGI(locationBlock, extension, binaryPath)) {

                    std::cout << "╔══════════════════════╗\n";
                    std::cout << "║         CGI          ║\n";
                    std::cout << "╚══════════════════════╝\n";

                    response = handleCgiGet(absolutePath, binaryPath, request);
                    std::string headers = response.first;
                    std::string body = response.second;

                    std::string contentType = extractContentType(headers);
                    std::string contentLength = ftToString(body.length());

                    std::size_t lastSlashPos = contentType.rfind('/');
                    std::string type = (lastSlashPos != std::string::npos) ? contentType.substr(lastSlashPos + 1) : "txt";

                    std::multimap<std::string, std::string> splitedHeaders = parseResponseHeaders(headers);

                    request.response = responseBuilder()
                        .addStatusLine("200")
                        .addContentType(type);
                    for (std::multimap<std::string, std::string>::iterator it = splitedHeaders.begin(); it != splitedHeaders.end(); it++) {
                        if (it->second.find("\r") != std::string::npos) {
                            it->second = it->second.substr(0, it->second.find("\r"));
                        }
                        if (it->second.find("\n") != std::string::npos) {
                            it->second = it->second.substr(0, it->second.find("\n"));
                        }
                        
                        if (it->first == "Status") {
                            std::string status = it->second;
                            std::stringstream ss(status);
                            std::string statusNumber;
                            ss >> statusNumber;

                            request.response.addStatusLine(statusNumber);
                        } else {
                            std::string key = lower(it->first);
                            if (key != "content-type" && key != "content-length")
                                request.response.addCustomHeader(it->first, it->second);
                        }
                    }
                    request.response.addResponseBody(body);
                    throw ("CGI");
                }
            }
        }

        std::fstream file(absolutePath.c_str());
        if ( file.good() ) {

            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;

            request.response = responseBuilder()
                .addStatusLine("200")
                .addContentType(absolutePath)
                .addResponseBody(content);
            throw "200";
        }
    }

    //* Index File does not exist, check for autoindex
    if (directives["autoindex"] == "on") {
        autoIndexFunction(root, request);
    } else {
        request.response = responseBuilder()
        .addStatusLine("403")
        .addContentType("text/html")
        .addResponseBody(request.getPageStatus(403));
        throw ("403");
    }

}

void getFile(std::string &absolutePath, std::string &uri, Request &request) {

    std::pair<std::string, std::string> response;
    size_t pos = uri.rfind('/');
    std::string file = uri.erase(0, pos);

    {
        if (file.find('.') != std::string::npos) {

            std::string extension = file.substr(file.find_last_of('.'));
            std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
            std::string binaryPath;

            if (isValidCGI(locationBlock, extension, binaryPath)) {

                std::cout << "╔══════════════════════╗\n";
                std::cout << "║         CGI          ║\n";
                std::cout << "╚══════════════════════╝\n";

                response = handleCgiGet(absolutePath, binaryPath, request);
                std::string headers = response.first;
                std::string body = response.second;

                std::string contentType = extractContentType(headers);
                std::string contentLength = ftToString(body.length());

                std::size_t lastSlashPos = contentType.rfind('/');
                std::string type = (lastSlashPos != std::string::npos) ? contentType.substr(lastSlashPos + 1) : "txt";

                std::multimap<std::string, std::string> splitedHeaders = parseResponseHeaders(headers);

                request.response = responseBuilder()
                    .addStatusLine("200")
                    .addContentType(type);
                for (std::multimap<std::string, std::string>::iterator it = splitedHeaders.begin(); it != splitedHeaders.end(); it++) {
                    if (it->second.find("\r") != std::string::npos) {
                        it->second = it->second.substr(0, it->second.find("\r"));
                    }
                    if (it->second.find("\n") != std::string::npos) {
                        it->second = it->second.substr(0, it->second.find("\n"));
                    }

                    if (it->first == "Status") {
                        std::string status = it->second;
                        std::stringstream ss(status);
                        std::string statusNumber;
                        ss >> statusNumber;

                        request.response.addStatusLine(statusNumber);
                    } else {
                        std::string key = lower(it->first);
                        if (key != "content-type" && key != "content-length")
                            request.response.addCustomHeader(it->first, it->second);
                    }
                }
                request.response.addResponseBody(body);

                throw ("CGI");
            }
        }

        std::fstream file(absolutePath.c_str());

        if ( file.good() ) {
            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            request.response = responseBuilder()
                .addStatusLine("200")
                .addContentType(absolutePath)
                .addContentLength(str)
                .addResponseBody(str);
            throw "200";
        } else {
            request.response = responseBuilder()
                .addStatusLine("500")
                .addContentType("text/html")
                .addResponseBody(request.getPageStatus(500));
            throw "500";
        }
    }
}

// void getMethod(Request &request) {

//     std::cout << "╔══════════════════════════╗\n";
//     std::cout << "║        GET Method        ║";std::cout << "\tURI: " << request.getUri() << "\n";
//     std::cout << "╚══════════════════════════╝\n";

//     std::string concatenateWithRoot;
//     retrieveRootAndUri(request, concatenateWithRoot);

//     std::string uri = request.getUri();
//     if (uri.find('?') != std::string::npos) {
//         parseQueriesInURI(request, uri);
//     }
//     uri = decodeUrl(uri);
//     request.setUri(uri);

// 	std::string result =  CheckPathForSecurity(concatenateWithRoot+uri);
// 	if (result.find(concatenateWithRoot) == std::string::npos) {
// 		request.response = responseBuilder()
//             .addStatusLine("403")
//             .addContentType("text/html")
//             .addResponseBody(request.getPageStatus(403));
//             throw "403 Security";
// 	}

//     concatenateWithRoot = result;
//     const char *path = concatenateWithRoot.c_str();
//     struct stat fileStat;

//     if ( stat(path, &fileStat) == 0 ) {
//         if (S_ISREG(fileStat.st_mode)) {
//             requestTypeFile(concatenateWithRoot, uri, request);
//         } else if (S_ISDIR(fileStat.st_mode)) {
//             requestTypeDirectory(concatenateWithRoot, uri, request);
//         } else {
//             request.response = responseBuilder()
//             .addStatusLine("500")
//             .addContentType("text/html")
//             .addResponseBody(request.getPageStatus(500));
//             throw "500";
//         }
//     } else if (uri == "/favicon.ico") {
//         std::ifstream file("./pages/response_pages/favicon.ico");
//         std::string content = (std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()));
//         request.response = responseBuilder()
//             .addStatusLine("200")
//             .addContentType("image/x-icon")
//             .addResponseBody(content);
//         throw "200";
//     } else {
//         request.response = responseBuilder()
//             .addStatusLine("404")
//             .addContentType("text/html")
//             .addResponseBody(request.getPageStatus(404));
//         throw "404";
//     }

// }
