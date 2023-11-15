#include "webserve.hpp"
#include <fstream>

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

    std::string response = "HTTP/1.1 200 OK \r\n"; request.setResponseVector(response);
    response = "Content-type: text/html\r\n"; request.setResponseVector(response);
    response = "\r\n"; request.setResponseVector(response);

    dir_ptr = opendir(absolutePath.c_str());
    if (dir_ptr == NULL) {
        std::cout << "Error: cannot open the file/directory\n"; 
        exit (1);
    }

    response = "<html><head><title>Index of " + request.getUri() + "</title></head><body><h1>Index \
         of " + request.getUri() + "</h1><hr><pre>"; request.setResponseVector(response);

    while ( (read_dir = readdir(dir_ptr)) != NULL ) {
        std::string link = read_dir->d_name;
        if (read_dir->d_type == DT_REG || read_dir->d_type == DT_DIR) {
            response = "<a href=" + link + "> "+ read_dir->d_name + "/ </a>\r\n" ; request.setResponseVector(response);
        }
    }

    response = "</pre><hr></body></html>"; request.setResponseVector(response);
    std::cout << response << std::endl;
    if (closedir(dir_ptr) == -1) {
        std::cout << "Error: cannot close the directory" << std::endl; 
        throw "Error: closedir()";
    }
    throw "200 autoindex";

}

void requestTypeDirectory(std::string &root, std::string &uri, Request &request) {

    std::string response;
    if ( ! request.getSaveLastBS() ) {

        response = "HTTP/1.1 301 Moved Permanently\r\n"; request.setResponseVector(response);
        response = "Location: " + uri + "/\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 21\r\n"; request.setResponseVector(response);
        response = "\r\n"; request.setResponseVector(response);
        response = "<html><h1>301 Moved Permanently</h1></html>\r\n"; request.setResponseVector(response);
        throw "301";
    }

    // std::map<std::string, std::string> directives = request.getDirectives();
    std::map<std::string, std::string> directives = request.getLocationBlockWillBeUsed();
    mapConstIterator it = directives.find("index");

    // for (auto it : directives) {
    //     std::cout << "BASED UPON|" << it.first << "|\t|" << it.second << "|\n";
    // }
    std::string absolutePath = root;

    //* Index file if it exists 
    if (it != directives.end()) {
        std::string indexFile = it->second;
        std::string extension = indexFile.substr(indexFile.find('.'), ( indexFile.length() - indexFile.find('.')) );

        if ( extension != ".html") {
            //! RUN CGI !
            throw "CGI";
        }

        response = "HTTP/1.1 200 OK \r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);

        absolutePath += '/' + indexFile;
        std::fstream file(absolutePath);

    
        if ( file.good() ) {

            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;

            response = "Content-Length: "; response += content.size(); response += "\r\n"; request.setResponseVector(response);
            response = "\r\n" ; request.setResponseVector(response);
            response = content ; request.setResponseVector(response);

            throw "200";
        }
    } else {
        std::cout << "Error: open indexFile has failed\n";
        //throw "502";
    }

    //* Index File doesn't exist && check autoindex
    //TODO : check if the '/' must be added to the absolute path;

    // absolutePath = root ;
    mapConstIterator autoIn = directives.find("autoindex");

    if (autoIn != directives.end() ) {
        autoIndexFunction(root, request);
    } else {
        response = "HTTP/1.1 403 Forbidden \r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 36\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1>403 Forbidden</h1></html>\r\n"; request.setResponseVector(response);
        response = "\r\n"; request.setResponseVector(response);
        throw ("403");
    }

}


void requestTypeFile(std::string &absolutePath, std::string &uri, Request &request) {

    std::string response;

    std::map<std::string, std::string> directives = request.getDirectives();
    size_t pos = uri.rfind('/');

    std::string file = uri.erase(0, pos);


    {
        std::string extension = file.substr(file.find('.'), ( file.length() - file.find('.')) );

        if ( extension == ".php" || extension == ".py") {
            //! RUN CGI !
            throw "CGI";
        }

        response = "HTTP/1.1 200 OK \r\n"; request.setResponseVector(response);
        response = "Content-type: text/plain; charset=UTF-8\r\n\r\n"; request.setResponseVector(response);

        std::fstream file(absolutePath);
    
        if ( file.good() ) {

            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;

            response = "Content-Length: "; response += content.size(); response += "\r\n"; request.setResponseVector(response);
            response = "\r\n" ; request.setResponseVector(response);
            response = content ; request.setResponseVector(response);

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

void getMethod(Request &request) {

    std::string concatenateWithRoot , locationUsed;

    retrieveRootAndUri(request, concatenateWithRoot, locationUsed);


    if ( concatenateWithRoot.empty() ) {

        mapConstIterator it = (request.getDirectives()).find("root");
        if (it == (request.getDirectives()).end()) {

            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"; request.setResponseVector(response);
            response = "Content-Length: 130\r\n\r\n" ; request.setResponseVector(response);
            response = "<html><head><title>Welcome to Our Webserver!</title></head>" ; request.setResponseVector(response);
            response = "<body><p><em>Thank you for using our webserver.</em></p></body></html>\r\n"; request.setResponseVector(response);

            throw "No Root: 200";
        }
        else
            concatenateWithRoot = it->second;

    }

    //?FIXED : if the uri doesn't have the exact location_match -> it's handled by the state system call 
    // the stat checks from the root of the file exists or no
    std::string uri = request.getUri();
    // std::cout << "URI |" << uri << "|\n";
    // std::cout << "ROOT |" << concatenateWithRoot << "|\n"; 
    concatenateWithRoot += uri;

    // std::cout << "BECOMES  |" << concatenateWithRoot << "|\n";

    const char *path = concatenateWithRoot.c_str();
    struct stat fileStat;

    if ( stat(path, &fileStat) == 0 ) {
        if (S_ISREG(fileStat.st_mode)) {
            requestTypeFile(concatenateWithRoot, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            requestTypeDirectory(concatenateWithRoot, uri, request);
        } else {
            std::string response = "HTTP/1.1 502 Bad Gateway\r\n"; request.setResponseVector(response);
            response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
            response = "Content-Length: 37\r\n\r\n"; request.setResponseVector(response);
            response = "<html><h1>502 Bad Gateway</h1></html>\r\n"; request.setResponseVector(response);
            throw "502"; 
        }
    } 
    else {
        
        std::string response = "HTTP/1.1 404 Not Found\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 36\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1> 404 Not Found</h1></html>\r\n"; request.setResponseVector(response);
        throw "4041"; 
    }

}
