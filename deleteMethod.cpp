#include "webserve.hpp"
#include <dirent.h>
#include <iostream>
#include <sys/types.h>
#include <string>

static bool checkCGI(Request &request) {

    for (vectorToMapIterator it = request.getLocationsBlock().begin(); it != request.getLocationsBlock().end(); ++it) {
        std::map<std::string, std::string> location = (*it);
        if ( location["location match"] == "/cgi-bin") {
            request.setCgiDirectives(location);
            return (true);
        }
    }

    return (false);
}

// static void removeAllFilesRecursively(Request &request, char *directory) {
//     DIR *dir_ptr;
//     std::string response;
//     struct dirent *read_dir;
//     dir_ptr = opendir(directory);
//     if ( ! strncmp(directory, ".", 2) || ! strncmp(directory, "..", 3 ) ) { return ; }
//     while ((read_dir = readdir(dir_ptr)) != NULL) {
//         if (read_dir->d_type == DT_DIR) {
//             if (! strncmp(directory, ".", 2) || ! strncmp(directory, "..", 3)) { continue ; }
//             removeAllFilesRecursively(request, read_dir->d_name);
//         } else {
//             remove(read_dir->d_name); 
//         }
//     }
// }

// static bool deleteAllFolderContent(Request &request,std::string &absolutePath) {
//     DIR *dir_ptr;
//     std::string response;
//     struct dirent *read_dir;
//     (void)request
    // if (absolutePath == "." || absolutePath == "..") {
    //     response = "HTTP/1.1 403 Forbidden\r\n"; request.setResponseVector(response);
    //     response = "Content-Type: text/html\r\nContent-Length: 72\r\n\r\n"; request.setResponseVector(response);
    //     response = "<html><h1>403 Forbidden Trying to delete the previous folder</h1></html>\r\n"; request.setResponseVector(response);
    //     throw "403";         
    // }
    // dir_ptr = opendir(absolutePath.c_str());
    // while ((read_dir = readdir(dir_ptr)) != NULL) {
    //     //* check for regular files
    //     remove(absolutePath.c_str()); 
    //     return (true);
    // }
//     if (stat ())
//     return (false);
// }

static bool deleteAllFolderContent(Request &request,std::string &absolutePath) {

    //* readdir -> compare -> remove -> closedir(return (1))
    DIR *dir_ptr;
    struct dirent *read_dir;

    // absolutePath += '/';

    dir_ptr = opendir(absolutePath.c_str());
    while ((read_dir = readdir(dir_ptr)) != NULL)
    {
        std::string cmp = read_dir->d_name; 
        std::string dir = absolutePath + '/' + cmp;
        if ( cmp.compare(".") && cmp.compare("..") ) {
            if (read_dir->d_type == DT_DIR) {
                if (!deleteAllFolderContent(request, dir))
                    return (false);
            }
            if ( remove(dir.c_str()) ) {
                closedir(dir_ptr);
                return (false);
            }
        } 
    }

    closedir(dir_ptr);
    return (true);
}


static void deleteDirectory(std::string &absolutePath, std::string &uri, Request &request) {

    std::string response; (void)uri;

    if ( ! request.getSaveLastBS() ) {

        response = "HTTP/1.1 409 Conflict\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\nContent-Length: 34\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1>409 Conflict</h1></html>\r\n"; request.setResponseVector(response);
        throw "409";        

    }

    // absolutePath += '/';

    //* if location has CGI 
    bool validCGI = false;
    for (mapConstIterator it = request.getLocationBlockWillBeUsed().begin() ; it != request.getLocationBlockWillBeUsed().end(); ++it ) {
        if (it->first == "fastcgi_pass") {
            validCGI = true ; break;
        }
    }

    if ( validCGI ) {
        std::map<std::string , std::string> direc = request.getLocationBlockWillBeUsed();
        std::string indexFile = direc["index"];
        if ( indexFile.size() ) {
            //! RUN CGI on requested file with DELTE REQUEST_METHOD
            throw "CGI";
        } else {
            response = "HTTP/1.1 403 Forbidden\r\n"; request.setResponseVector(response);
            response = "Content-Type: text/html\r\nContent-Length: 39\r\n\r\n"; request.setResponseVector(response);
            response = "<html><h1>403 Forbidden</h1></html>\r\n"; request.setResponseVector(response);
            throw "403"; 
        }
    } else {
        //* DELETE All the Directories
        //* Open -> readdir -> remove all files and the directory itself (check by access)


        bool check = deleteAllFolderContent(request, absolutePath);
        if ( check ) {
            std::string response = "HTTP/1.1 204 No Content\r\n"; request.setResponseVector(response);
            response = "Content-Type: text/html\r\nContent-Length: 0\r\n\r\n"; request.setResponseVector(response);
            throw "204";
        }
    
        if ( ! check ) {

            if ( access(absolutePath.c_str(), W_OK) == 0 ) {
                response = "HTTP/1.1 500 Internal Server Error\r\n"; request.setResponseVector(response);
                response = "Content-Type: text/html\r\nContent-Length: 49\r\n\r\n"; request.setResponseVector(response);
                response = "<html><h1>500 Internal Server Error</h1></html>\r\n"; request.setResponseVector(response);
                throw "500";
            } else {
                response = "HTTP/1.1 403 Forbidden\r\n"; request.setResponseVector(response);
                response = "Content-Type: text/html\r\nContent-Length: 39\r\n\r\n"; request.setResponseVector(response);
                response = "<html><h1>403 Forbidden</h1></html>\r\n"; request.setResponseVector(response);
                throw "403";
            }
        }

    }

}

static void deleteFile(std::string &absolutePath, std::string &uri, Request &request) {

    bool cgi = checkCGI(request); (void)uri;
    if ( ! cgi ) {
        //* DELETE THE FILE

        int out = std::remove(absolutePath.c_str());

        if ( out != 0 ) {
            std::string response = "HTTP/1.1 502 Bad Gateway\r\n"; request.setResponseVector(response);
            response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
            response = "Content-Length: 56\r\n\r\n"; request.setResponseVector(response);
            response = "<html><h1>Server Couldn't Delete the File</h1></html>\r\n"; request.setResponseVector(response);
            throw "Error(): remove system call";             

        } else {
            std::string response = "HTTP/1.1 204 No Content\r\n\r\n"; request.setResponseVector(response);
            response = "<html><h1>Server Couldn't Delete the File</h1></html>\r\n"; request.setResponseVector(response);
            throw "204";
        }

    } else {
        //! RUN CGI
        throw "CGI";
    }

}

void deleteMethod(Request &request) {

    std::string concatenateWithRoot, locationUsed;

    retrieveRootAndUri(request, concatenateWithRoot, locationUsed);
    // TODO : request.setRoot(concatenateWithRoot);

    if ( concatenateWithRoot.empty() ) {

        mapConstIterator it = (request.getDirectives()).find("root");
        if (it == request.getDirectives().end() ) {
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"; request.setResponseVector(response);
            response = "Content-Length: 130\r\n\r\n" ; request.setResponseVector(response);
            response = "<html><head><title>Welcome to Our Webserver!</title></head>" ; request.setResponseVector(response);
            response = "<body><p><em>Thank you for using our webserver.</em></p></body></html>\r\n"; request.setResponseVector(response);

            throw "No Root: 200";
        } else {
            concatenateWithRoot = it->second;
        }
    }

    std::string uri = request.getUri();
    request.setRoot(concatenateWithRoot);

    concatenateWithRoot += uri;
    const char *path = concatenateWithRoot.c_str();
    struct stat fileStat;

    if ( stat(path, &fileStat) == 0 ) {
        if (S_ISREG(fileStat.st_mode)) {
            deleteFile(concatenateWithRoot, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            deleteDirectory(concatenateWithRoot, uri, request);
        } else {
            std::string response = "HTTP/1.1 502 Bad Gateway\r\n"; request.setResponseVector(response);
            response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
            response = "Content-Length: 37\r\n\r\n"; request.setResponseVector(response);
            response = "<html><h1>502 Bad Gateway</h1></html>\r\n"; request.setResponseVector(response);
            throw "502"; 
        }
    } else {

        std::string response = "HTTP/1.1 404 Not Found\r\n"; request.setResponseVector(response);
        response = "Content-Type: text/html\r\n"; request.setResponseVector(response);
        response = "Content-Length: 36\r\n\r\n"; request.setResponseVector(response);
        response = "<html><h1> 404 Not Found</h1></html>\r\n"; request.setResponseVector(response);
        throw "4041";         
    }

}