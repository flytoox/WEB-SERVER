/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aait-mal <aait-mal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/07 12:18:26 by aait-mal          #+#    #+#             */
/*   Updated: 2024/03/07 12:27:11 by aait-mal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/cgi.hpp"

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

// Function to parse HTTP headers string into a map
std::multimap<std::string, std::string> parseResponseHeaders(const std::string& headers) {
    std::multimap<std::string, std::string> headersMap;
    std::istringstream iss(headers);

    std::string line;
    while (std::getline(iss, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            headersMap.insert(std::make_pair(key, value));
        }
    }

    return headersMap;
}

