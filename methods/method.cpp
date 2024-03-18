#include "../includes/webserve.hpp"

void method(Request &request, void (*fileFunc)(std::string &, std::string &, Request &), void (*folderFunc)(std::string &, std::string &, Request &)) {

    if (request.getHttpVerb() == "POST") {
        uploadRequestBody(request);
        std::map<std::string, std::string> locations = request.getLocationBlockWillBeUsed();

        std::map<std::string, std::string>::const_iterator itContentType = (request.getHttpRequestHeaders()).find("Content-Type");
        std::string value = "";
        if (itContentType != (request.getHttpRequestHeaders()).end()) {
            value = itContentType->second;
        }

        if (value == "multipart/form-data;") {
            multipartContentType(request);
            request.response = responseBuilder()
                .addStatusLine("201")
                .addContentType("text/html")
                .addLocationFile(locations["upload_store"])
                .addResponseBody(request.getPageStatus(201));
            throw "201" ;
        }
    }


    //* get_requested_resource()
    std::string root;
    retrieveRootAndUri(request, root);

    std::string uri = request.getUri();
    if (uri.find('?') != std::string::npos) {
        parseQueriesInURI(request, uri);
    }

    uri = decodeUrl(uri);
    request.setUri(uri);

    std::string absolutePath =  CheckPathForSecurity(root+uri);
	if (absolutePath.find(root) == std::string::npos) {
		request.response = responseBuilder()
            .addStatusLine("403")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(403));
        throw "403 Security";
	}
    if (request.getHttpVerb() == "DELETE") {
        if (absolutePath == root) {
            request.response = responseBuilder()
            .addStatusLine("405")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(405));
            throw "405";
        }
    }
    //TODO: fix this error http://localhost:1111/../../bin/ls the response don't get send

    const char *path = absolutePath.c_str();
    struct stat fileStat;

    if (stat(path, &fileStat ) == 0) {

        if (S_ISREG(fileStat.st_mode)) {
            fileFunc(absolutePath, uri, request);
        } else if (S_ISDIR(fileStat.st_mode)) {
            folderFunc(absolutePath, uri, request);
        } else {
            request.response = responseBuilder()
                .addStatusLine("502")
                .addContentType("text/html")
                .addResponseBody(request.getPageStatus(502));
            throw "502";
        }
    } else {
        request.response = responseBuilder()
            .addStatusLine("404")
            .addContentType("text/html")
            .addResponseBody(request.getPageStatus(404));
        throw "404";
    }
}