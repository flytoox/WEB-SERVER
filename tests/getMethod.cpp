#include "webserve.hpp"

void requestTypeFile(Request &request) {

    (void)request;

}

void getMethod(Request &request) {

    std::map<std::string, std::string> locationBlock = request.getLocationBlockWillBeUsed();
    std::map<std::string, std::string> directives = request.getDirectives();

    std::string concatenateWithRoot , locationUsed;

    for (mapConstIterator it = locationBlock.begin(); it != locationBlock.end(); ++it) {
        if (it->first == "root") {
            concatenateWithRoot = it->second;
        }
        if (it->first == "location match" ) {
            locationUsed = it->second;
        }
    }

    if ( concatenateWithRoot.empty() ) {

        mapConstIterator it = directives.find("root");
        if (it != directives.end())
            concatenateWithRoot = it->second;
        // else
        //     throw "IMPOSI-NO-ROOT";

    }

    concatenateWithRoot += locationUsed;

    DIR *dir_ptr;
    struct dirent *read_dir;
    
    dir_ptr = opendir(concatenateWithRoot.c_str());
    if (dir_ptr == NULL) {
        throw " Error: cannot open the file/directory ";
        std::cout << "Error: cannot open the file/directory\n"; 
        exit (1);
    }

    std::string uri = request.getUri();
    //* fetchFileOrDir used for defining the file or directory we are looking for ! 
    //? Example: /etc/passwd/test.html -> test.html
    std::string fetchFileOrDir = uri.erase(uri.find(locationUsed), locationUsed.length() - 1);

    while ((read_dir = readdir(dir_ptr)) != NULL) {
        if (read_dir->d_name == fetchFileOrDir) {
            if (read_dir->d_type == DT_REG) {
                requestTypeFile(request);
            } 
            // else if (read_dir->d_type == DT_DIR) {
            //     requestTypeDirectory();
            // }
        }
    }



}
