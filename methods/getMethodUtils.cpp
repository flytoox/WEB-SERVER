/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   getMethodUtils.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aait-mal <aait-mal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/07 12:28:49 by aait-mal          #+#    #+#             */
/*   Updated: 2024/03/07 12:48:29 by aait-mal         ###   ########.fr       */
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