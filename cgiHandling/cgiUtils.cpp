/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiUtils.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aait-mal <aait-mal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 18:00:04 by aait-mal          #+#    #+#             */
/*   Updated: 2024/02/26 18:02:58 by aait-mal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/cgi.hpp"

void handleTimeout(int signal) {
    // Handle timeout signal
    if (signal == SIGALRM) {
        std::cerr << "Child process timed out and is being terminated.\n";
        _exit(EXIT_FAILURE);
    }
}

std::map<std::string, std::string> parseHeaders(const std::string& headers) {
    std::map<std::string, std::string> headerMap;

    // Split the headers into lines
    std::istringstream headerStream(headers);
    std::string line;
    getline(headerStream, line);  // Read the first line separately

    // Set the first line as "HTTP_REQUEST" key with the value of the entire line
    headerMap["HTTP_REQUEST"] = line;

    // Process the rest of the lines
    while (getline(headerStream, line)) {
        // Find the position of the colon (':')
        size_t colonPos = line.find(':');

        // If a colon is found, extract the key and value
        if (colonPos != std::string::npos) {
            // Extract the key and value
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2);  // Skip the space after the colon

            // Standardize and uppercase the key
            for (size_t i = 0; i < key.length(); ++i) {
                key[i] = toupper(key[i]);
                if (key[i] == '-')
                    key[i] = '_';
            }

            // Add "HTTP_" prefix to the key except for specific headers
            if (key != "HTTP_USER_AGENT" && key != "HTTP_REFERER"
                && key != "CONTENT_LENGTH" && key != "CONTENT_TYPE"
                && key != "AUTH_TYPE" && key != "REMOTE_USER"
                && key != "REMOTE_IDENT") {
                key = "HTTP_" + key;
            }

            headerMap[key] = value;
        }
    }

    return headerMap;
}

std::map<std::string, std::string> fillEnv(const std::map<std::string, std::string>& headers) {
    std::map<std::string, std::string> envVars;

    // Set up the environment variables
    envVars["SERVER_SOFTWARE"] = "webserve";
    envVars["SERVER_NAME"] = "localhost";
    envVars["GATEWAY_INTERFACE"] = "CGI/1.1";
    envVars["SERVER_PROTOCOL"] = "HTTP/1.1";

    envVars["REDIRECT_STATUS"] = "1";

    for (const auto& entry : headers) {
        if (entry.first == "HTTP_REQUEST") {
            // Parse the HTTP_REQUEST header to extract relevant information
            std::istringstream requestStream(entry.second);
            std::vector<std::string> requestTokens;
            std::string token;

            // Tokenize the HTTP_REQUEST header
            while (requestStream >> token) {
                requestTokens.push_back(token);
            }

            // Set REQUEST_METHOD
            if (!requestTokens.empty()) {
                envVars["REQUEST_METHOD"] = requestTokens[0];
            }

            // Set REQUEST_URI and parse QUERY_STRING
            if (requestTokens.size() > 1) {
                envVars["REQUEST_URI"] = requestTokens[1];

                // Parse QUERY_STRING if it exists
                size_t queryStringPos = envVars["REQUEST_URI"].find('?');
                if (queryStringPos != std::string::npos) {
                    envVars["QUERY_STRING"] = envVars["REQUEST_URI"].substr(queryStringPos + 1);
                }
            }

            // Set SERVER_PROTOCOL
            if (requestTokens.size() > 2) {
                envVars["SERVER_PROTOCOL"] = requestTokens[2];
            }
        } else {
            envVars[entry.first] = entry.second;
        }
    }

    return envVars;
}
