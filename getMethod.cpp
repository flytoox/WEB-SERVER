#include "webserve.hpp"
#include <fstream>
#include <string>

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

    request.response = responseBuilder()
    .addStatusLine("200")
    .addContentType("text/html"); //* COOL

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
    request.response = responseBuilder().addResponseBody(response);
    if (closedir(dir_ptr) == -1) {
        std::cout << "Error: cannot close the directory" << std::endl; 
        throw "Error: closedir()";
    }
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

    std::string absolutePath = root;

    //* Index file if it exists 
    if (it != directives.end()) {
        std::string indexFile = it->second;
        std::string extension = indexFile.substr(indexFile.find('.'), ( indexFile.length() - indexFile.find('.')) );

        if ( extension != ".html") {
            //! RUN CGI !
            throw "CGI";
        }


        absolutePath += '/' + indexFile;
        std::fstream file(absolutePath);

    
        if ( file.good() ) {

            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;

            request.response = responseBuilder()
            .addStatusLine("200")
            .addContentType(absolutePath);
            request.response = responseBuilder().addResponseBody(content);
            throw "200";
        } else {
            request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html");
            request.response = responseBuilder().addResponseBody("<html><h1>400 Bad Request</h1></html>");
            throw "4001";
        }
    } 
    // else {
    //     //TODO: check this else below if it is valid
    //     std::cout << "Error: open indexFile has failed\n";
    //     //throw "502";
    // }

    //* Index File doesn't exist && check autoindex
    //TODO : check if the '/' must be added to the absolute path;

    // absolutePath = root ;
    mapConstIterator autoIn = directives.find("autoindex");

    std::string response;

    if (autoIn != directives.end() ) {
        autoIndexFunction(root, request);
    } else {
        request.response = responseBuilder()
        .addStatusLine("403")
        .addContentType("text/html")
        .addResponseBody("<html><h1>403 Forbidden</h1></html>");
        throw ("403");
    }

}


void requestTypeFile(std::string &absolutePath, std::string &uri, Request &request) {

    std::string response;

    std::map<std::string, std::string> directives = request.getDirectives();
    size_t pos = uri.rfind('/');

    std::string file = uri.erase(0, pos);


    {

        if ( file.find('.') != std::string::npos ) {

            std::string extension = file.substr(file.find('.'), ( file.length() - file.find('.')) );

            if ( extension == ".php" || extension == ".py") {
                //! RUN CGI !
                response = "HTTP/1.1 200 OK \r\n"; request.setResponseVector(response);
                response = "Content-type: text/html; charset=UTF-8\r\n\r\n"; request.setResponseVector(response);
                throw "CGI";
            }

        }
    
        std::fstream file(absolutePath);

        //TODO: FIX IF THE CONETENT IS VIDEO

        if ( file.good() ) {

            // std::cout << "GOOTOOT \n";

            // std::cout << "hhhhhere\n"    ;
            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;
            // std::cout << << content << std::endl;
            //std::string response = "HTTP/1.1 200 OK \r\n"; request.setResponseVector(response);
            //unsigned long number = content.size() ; std::ostringstream oss ; oss << number ; std::string result = oss.str();
            //response = "Content-Length: "; response += result ; response += "\r\n" ; request.setResponseVector(response);
            //response = "Content-Type: video/mp4\r\n\r\n"; request.setResponseVector(response);
            //response = "Content-Type: text/html\r\n\r\n"; request.setResponseVector(response);
            //response = content ; request.setResponseVector(response);
            // std::cerr << content << std::endl;
            //std::cout << request.getResponse << std::endl;

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
        if (it->first == "location match" ) {
            locationUsed = it->second;
        }
    }
}


void parseQueriesInURI(Request &request,std::string &uri) {

   // uri = /?username=sana&password=123

    std::string queriesString = uri.substr(uri.find('?') + 1); //username=sana&password=123
    std::map<std::string, std::string> mapTopush;

    std::vector<std::string> keyValueVector = splitString(queriesString, "&");

    for (const_vector_it it = keyValueVector.begin(); it != keyValueVector.end(); it++) {
        std::string keyValue = (*it);
        size_t signPos = keyValue.find('=');
        if (signPos != std::string::npos) {
            pair pair = std::make_pair(keyValue.substr(0, signPos), keyValue.substr(signPos + 1));
            mapTopush.insert(pair);
        } else {
            request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody("<html><body><h1>400 Bad Request</h1></body></html>");
            throw "400";
        }
    }


    // std::cout << "***TRY****"


    request.setUrlencodedResponse(mapTopush);

    // Remove queries from uri 
    uri.erase(uri.find('?'));
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
		result += s + "/";
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
            .addResponseBody("<html><head><title>Welcome to Our Webserver!</title></head><body><p><em>Thank you for using our webserver.</em></p></body></html>");

            throw "No Root: 200";
        }
        else
            concatenateWithRoot = it->second;

    }

    //?FIXED : if the uri doesn't have the exact location_match -> it's handled by the state system call 
    // the stat checks from the root of the file exists or no
    std::string uri = request.getUri();
    // std::cout << "BEFORE URI |" << uri << "|\n";
    if (uri.find('?') != std::string::npos) {
        parseQueriesInURI(request, uri);
    }


	//uri : /../../tmp/ll.txt
	//concatenateWithRoot : /Users/sizgunan/
	std::string result =  CheckPathForSecurity(concatenateWithRoot+uri);
	if (result.find(concatenateWithRoot) == std::string::npos) {
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody("<html><h1>403 Forbidden for Security Purposes</h1></html>");
            throw "403 Security"; 
	}
    // std::cout << "AFTER URI |" << uri << "|\n";
    // std::cout << "ROOT |" << concatenateWithRoot << "|\n"; 
    concatenateWithRoot += uri;
    std::cout << "GET: ABSOLUTEPATH|" << concatenateWithRoot << "|\n";


    const char *path = concatenateWithRoot.c_str();
    struct stat fileStat;

    // std::cout << "--> absolutePath |" << concatenateWithRoot << "|\n";
    // std::cout << "--> uri |" << uri << "|\n"; 

    // std::cout << "absolutePath:|" << concatenateWithRoot << "|\tURI|" << uri << "|\n";
    if ( stat(path, &fileStat) == 0 ) {
        if (S_ISREG(fileStat.st_mode)) {
            requestTypeFile(concatenateWithRoot, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            requestTypeDirectory(concatenateWithRoot, uri, request);
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
        throw "4041"; 
    }

}
