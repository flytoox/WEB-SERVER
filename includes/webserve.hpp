/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserve.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aait-mal <aait-mal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 17:24:49 by adnane            #+#    #+#             */
/*   Updated: 2024/03/24 22:02:25 by aait-mal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
// #include <endian.h>
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
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

//! Typedef
typedef std::map<std::string, std::string>::const_iterator               mapConstIterator;
typedef std::vector<std::string>::const_iterator                         const_vector_it;
typedef std::vector<std::map<std::string, std::string> >::const_iterator vectorToMapIterator;

//! multiplexing.cpp
void funcMultiplexingBySelect(configFile &configurationServers);

//! parseRequestHeader.cpp
void validateHeader(Request &request);

//! checkRequestedHttpMethod.cpp
void checkRequestedHttpMethod(Request &request);

//! getMethod.cpp
void getFolder(std::string &root, std::string &uri, Request &request);
void getFile(std::string &absolutePath, std::string &uri, Request &request);
void autoIndexFunction(std::string absolutePath, Request &request);
void retrieveRootAndUri(Request &request,std::string& concatenateWithRoot);
std::string CheckPathForSecurity(std::string path);

//! postMethod.cpp
void postFile(std::string &absolutePath, std::string &uri, Request &request);
void postFolder(std::string &root, std::string &uri, Request &request);
void parseQueriesInURI(Request &request,std::string &uri);
void uploadRequestBody(Request &request);

//! deleteMEthod.cpp
void deleteFolder(std::string &absolutePath, std::string &uri, Request &request);
void deleteFile(std::string &absolutePath, std::string &uri, Request &request);

//! parseRequestBody.cpp
void urlencodedContentType(Request &request);

//! receiveRequest.cpp
void receiveRequestPerBuffer(std::map<int, Request> &simultaneousRequests, int i, configFile &configurationServers, fd_set &allsd);
void reCheckTheServer(configFile &configurationServers, std::string &header, Request &request);
bool checkOverFlow(std::string &s);

//! methodsUtils.cpp
void method(Request &request, void (*fileFunc)(std::string &, std::string &, Request &), void (*folderFunc)(std::string &, std::string &, Request &));
