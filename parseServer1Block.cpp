#include "Server.hpp"
#include "configFile.hpp"


void    adjustServer1DirectivesMap(std::map<std::string, std::string> &serverDirectives) {

    serverDirectives["listen"] = "1111";
    serverDirectives["server_name"] = "localhost";
    serverDirectives["autoindex"] = "on";
    serverDirectives["root"] = "/Users/sizgunan";
    serverDirectives["client_max_body_size"] = "1000000";
    //serverDirectives["index"] = "index.html";

}

void    adjustServer1LocationBlockMap1(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/Documents";
    serverLocationDirectives["root"] = "/Users/sizgunan";
    serverLocationDirectives["autoindex"] = "on";
    //serverLocationDirectives["return"] = "Server1 Success";
    serverLocationDirectives["allowedMethods"] = "GET";

}

void    adjustServer1LocationBlockMap2(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/Desktop";
    serverLocationDirectives["index"] = "index.html";
    serverLocationDirectives["autoindex"] = "on";
    // serverLocationDirectives["return"] = "Server1 : Location : /Documents";

}

void    adjustServer1LocationBlockMap3(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/Desktop/webserv";
    serverLocationDirectives["index"] = "index.html";
    //serverLocationDirectives["autoindex"] = "on";
    // serverLocationDirectives["return"] = "Server1 : Location : /Desktop";
}

void adjustServer1LocationBlockVector
    (std::vector<std::map<std::string, std::string> > &server1LocationBlockVector) {


    std::map<std::string, std::string> location1;
    std::map<std::string, std::string> location2;
    std::map<std::string, std::string> location3;

    adjustServer1LocationBlockMap1(location1);
    adjustServer1LocationBlockMap2(location2);
    adjustServer1LocationBlockMap3(location3);


    server1LocationBlockVector.push_back(location1);
    server1LocationBlockVector.push_back(location2);
    server1LocationBlockVector.push_back(location3);

}
