#include "webserve.hpp"
#include <fstream>

// static void requestTypeFile(std::string &toFetch, Request &request) {



// }

// static void requestTypeDirectory(std::string &toFetch, Request &request) {

//     std::string res;

//     if ( ! request.getSaveLastBS() ) {

//         //TODO: Location Response Header must be set !
//         res = " 301 Moved Permanently + '/' added to the end";
//         request.setrequestOutputTest(res);
//         return ;
//     }

//     std::map<std::string, std::string> locations = request.getLocationBlockWillBeUsed();
//     mapConstIterator it = locations.find("index");
//     if ( it != locations.end()) {
//         //TODO: Add CGI if the file needs one

//         res = "200 OK and return the file input";
//         request.setrequestOutputTest(res);
//     }


// }

static void requestTypeDirectory(std::string &root, std::string &toFetch, Request &request) {

    std::string responseHeaders ;


    if ( ! (toFetch.empty()) && request.getSaveLastBS() == false ) {

        toFetch += '/';
        responseHeaders = "HTTP/1.1 301 Moved Permanently\r\n";
        request.setResponseMap(responseHeaders);
        responseHeaders = "Location: " + toFetch + "\r\n";
        request.setResponseMap(responseHeaders);
        responseHeaders = "Content-Length: 0\r\n";
        request.setResponseMap(responseHeaders);
        throw "301";
    }

    std::map<std::string , std::string> locations = request.getLocationBlockWillBeUsed();
    mapConstIterator it = locations.find("index");
    if (it != locations.end()) {
        std::string indexFile = it->second;
        std::string extension = indexFile.substr(indexFile.find('.'), ( indexFile.length() - indexFile.find('.')) );
        if ( extension != ".html") {
            //! RUN CGI !
            throw "CGI";
        }



        responseHeaders = "HTTP/1.1 200 OK \r\n";
        request.setResponseMap(responseHeaders);

        responseHeaders = "Content-type: text/html\r\n";
        request.setResponseMap(responseHeaders);

        //TODO: Pass from the root to the index.html
        std::string absolutePath = root + request.getUri() + '/' + indexFile;
        std::cout << "ABSOLUTE: |" << absolutePath << "|\n";

        std::fstream file(absolutePath);

        if ( file.good() ) {

            std::string str ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) ;
            std::string content = str;

            responseHeaders = "Content-Length: "; responseHeaders += content.size(); responseHeaders += "\r\n";
            request.setResponseMap(responseHeaders);
            responseHeaders = "\r\n";
            request.setResponseMap(responseHeaders);
            request.setResponseMap(content);
        }
    }
}

void getMethod(Request &request) {

    std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
    std::map<std::string, std::string> directives = request.getDirectives();

    std::string concatenateWithRoot , locationUsed, root;

    for (mapConstIterator it = locationBlock.begin(); it != locationBlock.end(); ++it) {
        if (it->first == "root") {
            concatenateWithRoot = root = it->second;
        }
        if (it->first == "location match" ) {
            locationUsed = root = it->second;
        }
    }

    std::cout << "root |" << concatenateWithRoot << "|\n";
    std::cout << "locationUsed |" << locationUsed << "|\n";


    if ( concatenateWithRoot.empty() ) {

        mapConstIterator it = directives.find("root");
        if (it != directives.end())
            concatenateWithRoot = root = it->second;
        // else
        //     throw "IMPOSI-NO-ROOT";

    }

    //* ROOT + LOCATION_MATCH
    concatenateWithRoot += locationUsed;

    std::cout << "Finally: |" << concatenateWithRoot << "|\n";

    std::string absolutePath = root + request.getUri() + '/';
    std::cout << "ABSOLUTE PATH : |" << absolutePath << "|\n";


    std::string uri = request.getUri();

    //* fetchFileOrDir used for defining the file or directory we are looking for ! 
    //? Example: /etc/passwd/test.html -> test.html

    // std::cout << "URI : |" << uri << "|\n";
    // std::cout << " location_Match: |" << locationUsed << "|\n";

    std::string fetchFileOrDir = uri.erase(uri.find(locationUsed), locationUsed.length() + 1);
    // std::cout << "Finally : |" << fetchFileOrDir << "|\n"; exit (0);

    DIR *dir_ptr;
    struct dirent *read_dir;
    
    dir_ptr = opendir(concatenateWithRoot.c_str());
    if (dir_ptr == NULL) {
        throw " Error: cannot open the file/directory ";
        std::cout << "Error: cannot open the file/directory\n"; 
        exit (1);
    }

    std::cout << "|" << fetchFileOrDir << "|\n";

    while ((read_dir = readdir(dir_ptr)) != NULL) {
        if (read_dir->d_name == fetchFileOrDir || fetchFileOrDir.empty() ) {
            // if (read_dir->d_type == DT_REG) {
            //     requestTypeFile(fetchFileOrDir, request);
            // }
            if (read_dir->d_type == DT_DIR || fetchFileOrDir.empty() ) {
                requestTypeDirectory(root, fetchFileOrDir, request);
                break;
            } 
            // else {
            //     initiateTheResponse()
            // }
        }
    }
}
