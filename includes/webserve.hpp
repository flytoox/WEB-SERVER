#pragma once

#include "configFile.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "multiplexing.hpp"
#include "responseBuilder.hpp"
#include "macros.hpp"
#include "cgi.hpp"

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

//! config.cpp


//! parseServer1Block.cpp
void    adjustServer1DirectivesMap(std::map<std::string, std::string> &serverDirectives);
void    adjustServer1LocationBlockMap1(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer1LocationBlockMap2(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer1LocationBlockMap3(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer1LocationBlockVector(std::vector<std::map<std::string, std::string> > &server1LocationBlockVector);

//! parseServer2Block.cpp
void    adjustServer2DirectivesMap(std::map<std::string, std::string> &serverDirectives);
void    adjustServer2LocationBlockMap1(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer2LocationBlockMap2(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer2LocationBlockMap3(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer2LocationBlockVector(std::vector<std::map<std::string, std::string> > &server2LocationBlockVector);

//! parseServer3Block.cpp

void    adjustServer3DirectivesMap(std::map<std::string, std::string> &serverDirectives);
void    adjustServer3LocationBlockMap1(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer3LocationBlockMap2(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer3LocationBlockMap3(std::map<std::string, std::string> &serverLocationDirectives);
void    adjustServer3LocationBlockVector(std::vector<std::map<std::string, std::string> > &server3LocationBlockVector);


//! multiplexing.cpp


void funcMultiplexingBySelect(configFile &configurationServers);

//! parseServer1Block.cpp | parseServer2Block.cpp | parseServer3Block.cpp


void    adjustServer1DirectivesMap(std::map<std::string, std::string> &serverDirectives);
void    adjustServer2DirectivesMap(std::map<std::string, std::string> &serverDirectives);
void    adjustServer3DirectivesMap(std::map<std::string, std::string> &serverDirectives);

void adjustServer1LocationBlockVector(std::vector<std::map<std::string, std::string> > &server1LocationBlockVector);
void adjustServer2LocationBlockVector(std::vector<std::map<std::string, std::string> > &server1LocationBlockVector);
void adjustServer3LocationBlockVector(std::vector<std::map<std::string, std::string> > &server1LocationBlockVector);




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
std::string CheckPathForSecurity(std::string path);

void getMethod(Request &request);

void requestTypeDirectory(std::string &root, std::string &uri, Request &request);
void requestTypeFile(std::string &absolutePath, std::string &uri, Request &request);
void retrieveRootAndUri(Request &request,std::string& concatenateWithRoot,std::string& locationUsed);
void getMethod(Request &request);
char hexToCharacters(const std::string& hex);
std::string decodeUrl(const std::string &srcString);

//! postMethod.cpp

void parseQueriesInURI(Request &request,std::string &uri);
void postMethod(Request &request);

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

//! cgi.cpp

std::pair<std::string, std::string> handleCgiGet(const std::string& file,
                                        const std::string& interpreterPath,
                                        Request &request);
std::pair<std::string, std::string> handleCgiPost(const std::string& file,
                                        const std::string& interpreterPath,
                                        Request &request);

bool isValidCGI(std::map<std::string, std::string> &directives, std::string &extension, std::string &cgiPath);
std::string extractContentType(const std::string& headers);
