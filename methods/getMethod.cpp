#include "../includes/webserve.hpp"

static void autoIndexFunction(std::string absolutePath, Request &request) {

    DIR *dir_ptr;
    struct dirent *read_dir;

    absolutePath += '/';
    std::string response = "";

    dir_ptr = opendir(absolutePath.c_str());
    if (dir_ptr == NULL) {
        std::cerr << "Error: opendir(): " << strerror(errno) << std::endl;
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(403));
            throw ("403");
    }

    response += "<html><head><title>Index of " + request.getUri() + "</title></head><body><h1>Index \
         of " + request.getUri() + "</h1><hr><pre>";

	while ( (read_dir = readdir(dir_ptr)) != NULL ) {
		std::string link = read_dir->d_name;
		if (read_dir->d_type == DT_REG) {
			response += "<a href= \"" + link  + "\"> "+ read_dir->d_name + " </a>\r\n" ;
		} else if (read_dir->d_type == DT_DIR) {
			response += "<a href= \"" + link  + "\"/> "+ read_dir->d_name + "/ </a>\r\n" ;
		}
	} 
    response += "</pre><hr></body></html>";

    if (closedir(dir_ptr) == -1) {
        std::cerr << "Error: closedir(): " << strerror(errno) << std::endl;
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(403));
            throw ("403");
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
            .addResponseBody(request.getPageStatus(301));
        throw "301";
    }
    std::map<std::string, std::string> directives = request.getLocationBlockWillBeUsed();
    mapConstIterator it = directives.find("index");

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
        .addResponseBody(request.getPageStatus(403));
        throw ("403");
    }

}

bool f(int ch) {
    return !std::isspace(ch);
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
            contentType.erase(contentType.begin(), std::find_if(contentType.begin(), contentType.end(), f));
            contentType.erase(std::find_if(contentType.rbegin(), contentType.rend(), f).base(), contentType.end());

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
    if (!directives.count("cgi_bin")) return false;
    std::vector<std::string> cgiParts = splitWithChar(directives["cgi_bin"], '\n');
    for (int i = 0; i < (int)cgiParts.size(); i++) {
        std::vector<std::string> cgiConfig = splitWhiteSpaces(cgiParts[i]);
        if (cgiConfig.size() < 2) continue;
        if (access(cgiConfig[0].c_str(), F_OK | X_OK) == -1) continue;
        for (int i = 1; i < (int)cgiConfig.size(); i++)
            if (cgiConfig[i] == extension) return (cgiPath = cgiConfig[0], true);
    }
    return false;
}


void requestTypeFile(std::string &absolutePath, std::string &uri, Request &request) {

    std::pair<std::string, std::string> response;
    size_t pos = uri.rfind('/');

    std::string file = uri.erase(0, pos);

    {
        if (file.find('.') != std::string::npos) {

            std::string extension = file.substr(file.find_last_of('.'));
            std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
            std::string binaryPath;

            if (isValidCGI(locationBlock, extension, binaryPath)) {
                std::cout << "\n\n\n\n\nCGI\n";
                response = handleCgiGet(absolutePath, binaryPath, request);

                std::string headers = response.first;
                std::string body = response.second;

                std::string contentType = extractContentType(headers);
                std::size_t lastSlashPos = contentType.rfind('/');
                std::string extension = (lastSlashPos != std::string::npos) ? contentType.substr(lastSlashPos + 1) : "";

                std::string contentLength = std::to_string(body.length());

                std::cout << "contentType: " << contentType << "\n";
                std::cout << "extension: " << extension << "\n";

                std::cout << "HEADERS |" << headers << "|\n";

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
            std::cout << "Success! \n";
            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;

            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType(absolutePath)
            .addContentLength(content)
            .addResponseBody(content);
            throw "200";
        } else {

            std::cout << "Failed! \n";
            request.response = responseBuilder()
                .addStatusLine("500")
                .addContentType("text/html")
                .addResponseBody(request.getPageStatus(500));
            throw "500";
        }
    }
}


void retrieveRootAndUri(Request &request,std::string& concatenateWithRoot) {

    std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
    mapConstIterator itRoot = locationBlock.find("root");

    if (itRoot == locationBlock.end()) {
        request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(418));
            throw ("200");
    }    

    concatenateWithRoot = (itRoot)->second;
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
	for (std::vector<std::string>::iterator s = ret.begin(); s != ret.end(); s++) {
		result += "/" + *s;
	}
	return result;
}

char hexToCharacters(const std::string& hex) {
    std::stringstream ss;
    ss << std::hex << hex;
    unsigned int c;
    ss >> c;
    return static_cast<char>(c);
}

std::string decodeUrl(const std::string &srcString) {
    std::string result;
    size_t length = srcString.size();

    for (size_t i = 0; i < length; ++i) {
        if (srcString[i] == '%' && i + 2 < length) {
            std::string hex = srcString.substr(i + 1, 2);
            result += hexToCharacters(hex);
            i += 2;
        } else if (srcString[i] == '+') {
            result += ' ';
        } else {
            result += srcString[i];
        }
    }
    return result;
}


void getMethod(Request &request) {

    std::string concatenateWithRoot;
    retrieveRootAndUri(request, concatenateWithRoot);

    std::string uri = request.getUri();
    if (uri.find('?') != std::string::npos) {
        uri.erase(uri.find('?'));
    }

    uri = decodeUrl(uri);
    request.setUri(uri);

	//uri : /../../tmp/ll.txt
	//concatenateWithRoot : /Users/sizgunan/
	std::string result =  CheckPathForSecurity(concatenateWithRoot+uri);
	if (result.find(concatenateWithRoot) == std::string::npos) {
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(403));
            throw "403 Security";
	}

    concatenateWithRoot = result;
    const char *path = concatenateWithRoot.c_str();
    struct stat fileStat;

    if ( stat(path, &fileStat) == 0 ) {
        if (S_ISREG(fileStat.st_mode)) {
            std::cout << "FILE \n";
            requestTypeFile(concatenateWithRoot, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            std::cout << "DIRECTORY \n";
            requestTypeDirectory(concatenateWithRoot, uri, request);
        } else {
            request.response = responseBuilder()
            .addStatusLine("500")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(500));
            throw "500";
        }
    } else if (uri == "/favicon.ico") {
        std::ifstream file("./response_pages/favicon.ico");
        std::string content = (std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()));
        request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType("image/x-icon")
            .addResponseBody(content);
        throw "200";
    } else {        
        request.response = responseBuilder()
            .addStatusLine("404")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(404));
        throw "4041";
    }

}
