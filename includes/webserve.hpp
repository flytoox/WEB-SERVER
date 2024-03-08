#pragma once

#include "configFile.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "multiplexing.hpp"
#include "responseBuilder.hpp"
#include "macros.hpp"
#include "cgi.hpp"
#include "utils.hpp"

#include <exception>
#include <string>
#include <map>
#include <fstream>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/_endian.h>
#include <stdexcept>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <set>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include<algorithm>
#include <sys/wait.h>
#include <signal.h>

#include <cstdio>
#include <sys/errno.h>

#include <dirent.h>
#include <sys/stat.h>

#include <fcntl.h>
//! Typedef


typedef std::map<std::string, std::string>::const_iterator               mapConstIterator;
typedef std::vector<std::string>::const_iterator                         const_vector_it;
typedef std::vector<std::map<std::string, std::string> >::const_iterator vectorToMapIterator;

//! multiplexing.cpp

void funcMultiplexingBySelect(configFile &configurationServers);

//! re_configureRequestClass.cpp
//TODO: remove the requestOutput after checking the output request

//void parseRequestPerBuffer(std::string &buffer, std::string &requestOutput);
std::string &parseRequestPerBuffer(std::string &buffer, std::string &requestOutput);
void configureTheHttpHeaderRequestClass(std::string &header, Request &request);



// //! parseRequest.cpp

// void checkRequestFormat(Request &request, std::string &requestOutput);
// std::string &fetchLocation(std::string &uri, Request &request);


//! parseRequestHeader.cpp


void parseAndSetRequestHeader(Request &request);
void validateRequest(Request &request);


//! parseRequestBody.cpp

std::vector<std::string> splitString(const std::string& input, const std::string& delimiter);


//! parseRequestBody.cpp


void parseRequestBody(Request &request);


//! validateRequest.cpp

//! checkHttpMethod.cpp


void checkRequestedHttpMethod(Request &request);

//! getMethod.cpp
void getMethod(Request &request);
std::string CheckPathForSecurity(std::string path);
void autoIndexFunction(std::string absolutePath, Request &request);

void requestTypeDirectory(std::string &root, std::string &uri, Request &request);
void requestTypeFile(std::string &absolutePath, std::string &uri, Request &request);
void retrieveRootAndUri(Request &request,std::string& concatenateWithRoot);

//! postMethod.cpp

void parseQueriesInURI(Request &request,std::string &uri);
void postMethod(Request &request);
void uploadRequestBody(Request &request);

//! deleteMEthod.cpp

void deleteMethod(Request &request);

//! parseRequestBody.cpp

int hexaToDec(std::string &res);
void chunkedRequest(Request &request);
void textContentType(Request &request);
void pureBinary(std::string &image, std::string &destination);
void multipartContentType(Request &request);
void urlencodedContentType(Request &request);

//GetConfig
std::vector<std::string> splitWithChar(std::string s, char delim);

//! receiveRequest.cpp

void receiveRequestPerBuffer(std::map<int, Request> &simultaneousRequests, int i, configFile &configurationServers, fd_set &allsd);
void reCheckTheServer(configFile &configurationServers, std::string &header, Request &request);