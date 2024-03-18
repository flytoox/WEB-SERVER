/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   methodsUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aait-mal <aait-mal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/07 12:28:49 by aait-mal          #+#    #+#             */
/*   Updated: 2024/03/13 16:05:02 by aait-mal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserve.hpp"

void autoIndexFunction(std::string absolutePath, Request &request) {

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

void parseQueriesInURI(Request &request,std::string &uri) {
    
    //* Protect the queries if they exist
    if (request.getHttpVerb() == "DELETE") {
        uri.erase(uri.find('?'));
        return ;
    }
    size_t pos = uri.find('?');
    if (uri.length() == pos + 1)
        return ;

    std::string queriesString = uri.substr(uri.find('?') + 1);
    request.setQueryString(queriesString);
    std::map<std::string, std::string> mapTopush;

    std::stringstream ss(queriesString);
    std::vector<std::string> keyValueVector;
    std::string token;

    while (std::getline(ss, token, '&')) {
        keyValueVector.push_back(token);
    }

    for (const_vector_it it = keyValueVector.begin(); it != keyValueVector.end(); it++) {
        std::string keyValue = (*it);
        size_t signPos = keyValue.find('=');
        try {
            if (keyValue[0] == '?') {
                throw (std::runtime_error("400"));
            }
            if (signPos != std::string::npos) {
                if (keyValue.substr(signPos + 1).empty())
                    throw (std::runtime_error("400"));
                pair pair = std::make_pair(keyValue.substr(0, signPos), keyValue.substr(signPos + 1));
                mapTopush.insert(pair);
            } else {
                throw (std::runtime_error("400"));
            }
        } catch (std::exception &e) {
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody("<html><body><h1>400 Bad Request123</h1></body></html>");
            throw "400";
        }
    }


    request.setUrlencodedResponse(mapTopush);

    // Remove queries from uri
    uri.erase(uri.find('?'));
}

void uploadRequestBody(Request &request) {

    std::map<std::string, std::string>::const_iterator itContentType;

    itContentType = (request.getHttpRequestHeaders()).find("Content-Type");
    if ( itContentType != (request.getHttpRequestHeaders()).end()) {

        std::string value = itContentType->second;
        if (value == "text/plain") {
            std::string ret = request.getRequestBody();
            request.response = responseBuilder()
                .addStatusLine("200")
                .addContentType("text/html")
                .addResponseBody(ret);
            throw("textContentType");
        } else if (value == "application/x-www-form-urlencoded") {
            urlencodedContentType(request);
        } else if (value != "multipart/form-data;") {
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody(request.getPageStatus(400));
            throw "400 CT";
        }
    }
}

std::string lower(std::string str) {
    std::string ret = "";
    for (std::string::iterator it = str.begin(); it != str.end(); it++) {
        ret += std::tolower(*it);
    }
    return ret;
}