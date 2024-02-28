#include "webserve.hpp"
#include <dirent.h>
#include <iostream>
#include <sys/types.h>
#include <string>

// static bool checkCGI(Request &request) {

//     for (vectorToMapIterator it = request.getLocationsBlock().begin(); it != request.getLocationsBlock().end(); ++it) {
//         std::map<std::string, std::string> location = (*it);
//         if ( location["location"] == "/cgi-bin") {
//             request.setCgiDirectives(location);
//             return (true);
//         }
//     }

//     return (false);
// }

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

static void errorOccurredOnDeleteion(Request &request, std::string path) {

    if ( access(path.c_str(), W_OK) == 0 ) {

        request.response = responseBuilder()
        .addStatusLine("500")
        .addContentType("text/html")
        .addResponseBody("<html><h1>500 Internal Server Error</h1></html>");
        throw "500";
    } else {

        request.response = responseBuilder()
        .addStatusLine("403")
        .addContentType("text/html")
        .addResponseBody("<html><h1>403 Forbidden</h1></html>");
        throw "403";
    }

}


static std::string deleteAllFolderContent(Request &request,std::string &absolutePath, int frame) {

    //* readdir -> compare -> remove -> closedir(return (1))
    DIR *dir_ptr;
    struct dirent *read_dir;
    std::string checkFail;

    // absolutePath += '/';

    dir_ptr = opendir(absolutePath.c_str());
    if (!dir_ptr)
        errorOccurredOnDeleteion(request, absolutePath);
    while ((read_dir = readdir(dir_ptr)) != NULL)
    {
        std::string cmp = read_dir->d_name; 
        std::string dir = absolutePath + '/' + cmp;
        if ( cmp.compare(".") && cmp.compare("..") ) {
            if (read_dir->d_type == DT_DIR) {
                checkFail = deleteAllFolderContent(request, dir, frame + 1);
                if ( ! checkFail.empty() ) {
                    //* SMART MOVE
                    closedir(dir_ptr);
                    if (!frame)
                        errorOccurredOnDeleteion(request, checkFail);
                    return (checkFail);
                }
            }
            if ( remove(dir.c_str()) ) {
                //* false is returned which means that all files are removed within this folder, and it's time now to remove the folder itself
                closedir(dir_ptr);
                return (dir);
            }
        } 
    }
    closedir(dir_ptr);
    return ("");
}



static void deleteDirectory(std::string &absolutePath, std::string &uri, Request &request) {

    std::string response; (void)uri;

    if ( ! request.getSaveLastBS() ) {

        request.response = responseBuilder()
        .addStatusLine("409")
        .addContentType("text/html")
        .addResponseBody("<html><h1>409 Conflict</h1></html>");
        throw "409";        

    }

    // absolutePath += '/';

    // //* if location has CGI 
    // bool validCGI = false;
    // for (mapConstIterator it = request.getLocationBlockWillBeUsed().begin() ; it != request.getLocationBlockWillBeUsed().end(); ++it ) {
    //     if (it->first == "fastcgi_pass") {
    //         validCGI = true ; break;
    //     }
    // }

    // if ( validCGI ) {
    //     std::map<std::string , std::string> direc = request.getLocationBlockWillBeUsed();
    //     std::string indexFile = direc["index"];
    //     if ( indexFile.size() ) {
    //         //! RUN CGI on requested file with DELTE REQUEST_METHOD
    //         throw "CGI";
    //     } else {

    //         request.response = responseBuilder()
    //         .addStatusLine("403")
    //         .addContentType("text/html")
    //         .addResponseBody("<html><h1>403 Forbidden</h1></html>");
    //         throw "403"; 
    //     }
    // } else {
        //* DELETE All the Directories
        //* Open -> readdir -> remove all files and the directory itself (check by access)


        std::string check = deleteAllFolderContent(request, absolutePath, 0);
        if ( check.empty() ) {
            request.response = responseBuilder()
            .addStatusLine("204")
            .addContentType("text/html")
            .addResponseBody("<html><h1>204 No Content</h1></html>");
            throw "55204";
        }

    // }

}

static void deleteFile(std::string &absolutePath, std::string &uri, Request &request) {

    // bool cgi = checkCGI(request); 
    (void)uri;
    // if ( ! cgi ) {
        //* DELETE THE FILE

        int out = std::remove(absolutePath.c_str());

        if ( out != 0 ) {
            request.response = responseBuilder()
            .addStatusLine("502")
            .addContentType("text/html")
            .addResponseBody("<html><h1>Server Couldn't Delete the File</h1></html>");
            throw "Error(): remove system call";             

        } else {
            request.response = responseBuilder()
            .addStatusLine("204")
            .addContentType("text/html")
            .addResponseBody("<html><h1>204 No Content</h1></html>");
            throw "204";
        }

    // } 
    // else {
    //     //! RUN CGI
    //     throw "CGI";
    // }

}

void deleteMethod(Request &request) {

    std::string concatenateWithRoot, locationUsed;

    retrieveRootAndUri(request, concatenateWithRoot, locationUsed);



    //NEW SET 
    std::string uri = request.getUri();
    // std::cout << "BEFORE URI |" << uri << "|\n";
    if (uri.find('?') != std::string::npos) {
        parseQueriesInURI(request, uri);
    }

    // concatenateWithRoot += uri;



    // TODO : request.setRoot(concatenateWithRoot);
    // TODO : Figure out whta's that if statement for below 


    // if ( concatenateWithRoot.empty() ) {

    //     mapConstIterator it = (request.getDirectives()).find("root");
    //     if (it == request.getDirectives().end() ) {

    //         request.response = responseBuilder()
    //         .addStatusLine("200")
    //         .addContentType("text/html")
    //         .addResponseBody("<html><head><title>Welcome to Our Webserver!</title></head><body><p><em>Thank you for using our webserver.</em></p></body></html>");

    //         throw "No Root: 200";
    //     } else {
    //         concatenateWithRoot = it->second;
    //     }
    // }
	std::string result =  CheckPathForSecurity(concatenateWithRoot+uri);
	if (result.find(concatenateWithRoot) == std::string::npos) {
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody("<html><h1>403 Forbidden for Security Purposes</h1></html>");
            throw "403 Security"; 
	}
    // request.setRoot(concatenateWithRoot);
    // exit (0);

    // std::string uri = request.getUri();

    // std::cout << "URI|" << uri << "|\n";
    // std::cout << "root|" << concatenateWithRoot << "|\n";

    // concatenateWithRoot += uri;
    std::string absolutePath;
    absolutePath = result;

    // std::cout << "DELETE absolutePath|" << absolutePath << "|\n";

    std::cout << "DELETE  URI |" << uri << "|\n";
    std::cout << "DELETE concatenateWithRoot |" << concatenateWithRoot << "|\n";
    std::cout << "ABSOLUTE PATH |" << result << "|\n";
    // exit (0);
    if (absolutePath == concatenateWithRoot) {

        request.response = responseBuilder()
        .addStatusLine("405")
        .addContentType("text/html")
        .addResponseBody("<html><h1> 452 Method Not Allowed </h1></html>");
        throw "405";
    }

    const char *path = absolutePath.c_str();
    struct stat fileStat;

    if ( stat(path, &fileStat) == 0 ) {
        if (S_ISREG(fileStat.st_mode)) {
            deleteFile(absolutePath, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            deleteDirectory(absolutePath, uri, request);
        } else {

            request.response = responseBuilder()
            .addStatusLine("502")
            .addContentType("text/html")
            .addResponseBody("<html><h1>502 Bad Gateway</h1></html>");
            throw "DELETE 502"; 
        }
    } else {

        request.response = responseBuilder()
        .addStatusLine("404")
        .addContentType("text/html")
        .addResponseBody("<html><h1> 404 Not Found</h1></html>");
        throw "4041";         
    }

}