#include "../includes/webserve.hpp"

// static void fetchFullPath(std::string &serverName, std::string &listen, Request &request) {

//     std::map<std::string, std::string> directories;

//     directories = request.getDirectives();
//     mapConstIterator it = directories.find("server_name");

//     if ( it != directories.end() ) {
//         serverName = it->second;
//     } else {
//         std::cout << "Error: NO server_name \n"; exit (1);
//     }

//     it = directories.find("listen");
//     if ( it != directories.end() ) {
//         listen = it->second;
//     } else {
//         std::cout << "Error: NO listen \n"; exit (1);
//     }

// }

static void autoIndexFunction(std::string absolutePath, Request &request) {

    DIR *dir_ptr;
    struct dirent *read_dir;

    absolutePath += '/';
    std::string response = "";

    dir_ptr = opendir(absolutePath.c_str());
    if (dir_ptr == NULL) {
        std::cout << "Error: cannot open the file/directory\n";
        exit (1);
    }

    response += "<html><head><title>Index of " + request.getUri() + "</title></head><body><h1>Index \
         of " + request.getUri() + "</h1><hr><pre>";

    while ( (read_dir = readdir(dir_ptr)) != NULL ) {
        std::string link = read_dir->d_name;
        if (read_dir->d_type == DT_REG || read_dir->d_type == DT_DIR) {
            response += "<a href=" + link + "> "+ read_dir->d_name + "/ </a>\r\n" ;
        }
    }

    response += "</pre><hr></body></html>";
    if (closedir(dir_ptr) == -1) {
        std::cout << "Error: cannot close the directory" << std::endl;
        throw "Error: closedir()";
    }

    request.response = responseBuilder()
    .addStatusLine("200")
    .addContentType("text/html")
    .addResponseBody(response);
    throw "200 autoindex";

}

void requestTypeDirectory(std::string &root, std::string &uri, Request &request) {

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

    // for (auto it : directives) {
    //     std::cout << "ARE YOU IN|" << it.first << "| |" << it.second <<"|\n";
    // }
    // exit (0);

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
            std::cerr << "ABSOLUTE PATH: " << absolutePath << std::endl;
            std::string extension = absolutePath.substr(absolutePath.find_last_of('.'));
            std::pair<std::string, std::string> response;

            if ( extension != ".html") {
                std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
                std::string binaryPath;

                if (isValidCGI(locationBlock, extension, binaryPath)) {
                    std::cout << "\n\n\n\n\nCGI\n";
                    response = handleCgiGet(absolutePath, binaryPath, request);

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
                    // std::cout << "BODY |" << body << "|\n";

                    // Set the initial HTTP response headers
                    request.response = responseBuilder()
                    .addStatusLine("200")
                    .addContentType(extension)
                    .addResponseBody(body);
                    throw ("CGI");
                }
            }
        }


        // absolutePath += '/' + indexFile;
        std::fstream file(absolutePath);


        if ( file.good() ) {

            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;

            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType(absolutePath)
            .addResponseBody(content);
            throw "200";
        }
        // else {
        //     request.response = responseBuilder()
        //     .addStatusLine("400")
        //     .addContentType("text/html")
        //     .addResponseBody("<html><h1>400 Bad Request</h1><h3>CGI not set for the file</h3></html>");
        //     throw "4001";
        // }
    }
    // else {
    //     //TODO: check this else below if it is valid
    //     std::cout << "Error: open indexFile has failed\n";
    //     //throw "502";
    // }

    //* Index File doesn't exist && check autoindex
    //TODO : check if the '/' must be added to the absolute path;

    // absolutePath = root ;
    // mapConstIterator autoIn = directives.find("autoindex");
    // if autoIn.seconf == o

    if (directives["autoindex"] == "on") {
        autoIndexFunction(root, request);
    } else {
        request.response = responseBuilder()
        .addStatusLine("403")
        .addContentType("text/html")
        .addResponseBody("<html><h1>403 Forbidden</h1></html>");
        throw ("403");
    }

}

std::string extractContentType(const std::string& headers) {
    std::istringstream iss(headers);
    std::string line;

    while (std::getline(iss, line)) {
        // Convert the line to lowercase for case-insensitive comparison
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        // Find the line that contains "content-type:"
        std::size_t found = line.find("content-type:");
        if (found != std::string::npos) {
            // Extract the value after ":"
            std::string contentType = line.substr(found + 13);  // 13 is the length of "content-type:"

            // Find the position of the semicolon
            std::size_t semicolonPos = contentType.find(';');

            // Trim leading and trailing whitespaces
            contentType.erase(contentType.begin(), std::find_if(contentType.begin(), contentType.end(), [](int ch) {
                return !std::isspace(ch);
            }));
            contentType.erase(std::find_if(contentType.rbegin(), contentType.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), contentType.end());

            // Extract only the content type value until the semicolon if it exists
            if (semicolonPos != std::string::npos) {
                contentType = contentType.substr(0, semicolonPos - 1);
            }

            return contentType;
        }
    }

    return "";  // Return an empty string if "Content-Type" is not found
}

static std::vector<std::string> splitWhiteSpaces(std::string s) {
	std::stringstream ss(s);
	std::vector<std::string> v;
	std::string word;
	while (ss >> word)
		v.push_back(word);
	return (v);
}

bool isValidCGI(std::map<std::string, std::string> &directives, std::string &extension, std::string &cgiPath) {
    // std::cout << "BEFORE COUNT\n";
    if (!directives.count("cgi_bin")) return false;
    std::vector<std::string> cgiParts = splitWithChar(directives["cgi_bin"], '\n');
    // std::cout << "AFTER COUNT\n";
    for (int i = 0; i < (int)cgiParts.size(); i++) {
        std::vector<std::string> cgiConfig = splitWhiteSpaces(cgiParts[i]);
        if (cgiConfig.size() < 2) continue;
        if (access(cgiConfig[0].c_str(), F_OK | X_OK) == -1) continue;
        for (int i = 1; i < (int)cgiConfig.size(); i++)
            if (cgiConfig[i] == extension) return (cgiPath = cgiConfig[0], true);
    }
    // std::cout << "END of IS VALIDCGI\n";
    return false;
}


void requestTypeFile(std::string &absolutePath, std::string &uri, Request &request) {

    std::pair<std::string, std::string> response;
    size_t pos = uri.rfind('/');

    std::string file = uri.erase(0, pos);
    std::cout << "file " << file << std::endl;

    {
        if (file.find('.') != std::string::npos) {

            std::string extension = file.substr(file.find_last_of('.'));
            std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
            // std::cerr << "LOCATION BLOCK\n";
            // for (auto &i : locationBlock) std::cerr << i.first << ' ' << i.second << std::endl;
            // std::cerr << "END\n";
            std::string binaryPath;

            if (isValidCGI(locationBlock, extension, binaryPath)) {
                std::cout << "\n\n\n\n\nCGI\n";
                response = handleCgiGet(absolutePath, binaryPath, request);

                if (response.second == "No input file specified.\n") {
                    request.response = responseBuilder()
                    .addStatusLine("404")
                    .addContentType("text/html")
                    .addResponseBody("<html><h1>404 Not Found</h1><h2>Index file not found</h2></html>");
                    throw "404";
                }

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
                // std::cout << "BODY |" << body << "|\n";

                // Set the initial HTTP response headers
                request.response = responseBuilder()
                .addStatusLine("200")
                .addContentType(extension)
                .addResponseBody(body);
                throw ("CGI");
            }
        }

        std::fstream file(absolutePath);

        //TODO: FIX IF THE CONETENT IS VIDEO

        if ( file.good() ) {
            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;

            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType(absolutePath)
            .addContentLength(content)
            .addResponseBody(content);
            throw "200";
        }
    }
}


void retrieveRootAndUri(Request &request,std::string& concatenateWithRoot,std::string& locationUsed) {

    std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();

    for (mapConstIterator it = locationBlock.begin(); it != locationBlock.end(); ++it) {
        if (it->first == "root") {
            concatenateWithRoot = it->second;
        }
        if (it->first == "location" ) {
            locationUsed = it->second;
        }
    }
}

std::vector<std::string> splitWithChar(std::string s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss (s);
	std::string item;

	while (getline (ss, item, delim)) {
		result.push_back (item);
	}

	return result;
}

std::string CheckPathForSecurity(std::string path) {
	std::vector<std::string> ret = splitWithChar(path, '/');
	std::string result = "";

	for (int i = 0; i < (int)ret.size(); i++) {
		if (ret[i] == "..") {
			if (i) {
				ret.erase(ret.begin() + i);
				ret.erase(ret.begin() + i - 1);
				i -= 2;
			}
			else {
				ret.erase(ret.begin());
				i--;
			}
		} else if (ret[i] == ".") {
			ret.erase(ret.begin() + i);
			i--;
		}
	}
	for (std::string s : ret) {
		result += "/" + s;
	}

	return result;
}

void getMethod(Request &request) {

    std::string concatenateWithRoot , locationUsed;

    retrieveRootAndUri(request, concatenateWithRoot, locationUsed);

    if ( concatenateWithRoot.empty() ) {

        mapConstIterator it = (request.getDirectives()).find("root");
        if (it == (request.getDirectives()).end()) {

            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType("text/html")
            .addResponseBody("<html><head>Welcome to Our Webserver!</head><body><p><em>Thank you for using our webserver.</em></p></body></html>");

            throw "No Root: 200";
        }
        concatenateWithRoot = it->second;
    }

    //?FIXED : if the uri doesn't have the exact location_match -> it's handled by the state system call
    // the stat checks from the root of the file exists or no
    std::string uri = request.getUri();
    // std::cout << "BEFORE URI |" << uri << "|\n";
    if (uri.find('?') != std::string::npos) {
        uri.erase(uri.find('?'));
        // parseQueriesInURI(request, uri);
    }


	//uri : /../../tmp/ll.txt
	//concatenateWithRoot : /Users/sizgunan/
	std::string result =  CheckPathForSecurity(concatenateWithRoot+uri);
    // std::cout << "WHAT ROOT|" << concatenateWithRoot << "|\n";
    // std::cout << "WHAT CHECK|" << result << "|\n";
	if (result.find(concatenateWithRoot) == std::string::npos) {
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody("<html><h1>403 Forbidden for Security Purposes</h1></html>");
            throw "403 Security";
	}
    // std::cout << "AFTER URI |" << uri << "|\n";
    // std::cout << "ROOT |" << concatenateWithRoot << "|\n";
    // concatenateWithRoot += uri;
    concatenateWithRoot = result;
    // std::cout << "GET: ABSOLUTEPATH|" << concatenateWithRoot << "|\n";


    const char *path = concatenateWithRoot.c_str();
    struct stat fileStat;

    // std::cout << "--> absolutePath |" << concatenateWithRoot << "|\n";
    // std::cout << "--> uri |" << uri << "|\n";

    // std::cout << "absolutePath:|" << concatenateWithRoot << "|\tURI|" << uri << "|\n";
    if ( stat(path, &fileStat) == 0 ) {
        if (S_ISREG(fileStat.st_mode)) {
            std::cout << "IT'S FILE\n";
            requestTypeFile(concatenateWithRoot, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            std::cout << "IT'S DIRECTORY\n";
            requestTypeDirectory(concatenateWithRoot, uri, request);
        } else {
            request.response = responseBuilder()
            .addStatusLine("500")
            .addContentType("text/html")
            .addResponseBody("<html><h1>500 Internal Server Error</h1></html>");
            throw "500";
        }
    } else {

        request.response = responseBuilder()
        .addStatusLine("404")
        .addContentType("text/html")
        .addResponseBody("<html><h1> 404 Not Found</h1></html>");
        throw "4041";
    }

}
