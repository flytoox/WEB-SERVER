#include "Server.hpp"
#include "configFile.hpp"



void    adjustServer3DirectivesMap(std::map<std::string, std::string> &serverDirectives) {

    serverDirectives["listen"] = "3333";
    serverDirectives["server_name"] = "localhost";
    serverDirectives["autoindex"] = "on";
    serverDirectives["root"] = "/Users/sizgunan";
    serverDirectives["client_max_body_size"] = "3000000";

}

void    adjustServer3LocationBlockMap1(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/";
    serverLocationDirectives["root"] = "/Users/sizgunan";
    serverLocationDirectives["autoindex"] = "on";
    // serverLocationDirectives["return"] = "Server1 Success";

}

void    adjustServer3LocationBlockMap2(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/Documents";
    serverLocationDirectives["root"] = "/Users/sizgunan";
    serverLocationDirectives["autoindex"] = "on";
    // serverLocationDirectives["return"] = "Server3 : Location : /Documents";

}

void    adjustServer3LocationBlockMap3(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/Desktop";
    serverLocationDirectives["autoindex"] = "on";
    // serverLocationDirectives["return"] = "Server3 : Location : /Desktop";
}


void adjustServer3LocationBlockVector
    (std::vector<std::map<std::string, std::string> > &server3LocationBlockVector) {


    std::map<std::string, std::string> location1;
    std::map<std::string, std::string> location2;
    std::map<std::string, std::string> location3;

    adjustServer3LocationBlockMap1(location1);
    adjustServer3LocationBlockMap2(location2);
    adjustServer3LocationBlockMap3(location3);


    server3LocationBlockVector.push_back(location1);
    server3LocationBlockVector.push_back(location2);
    server3LocationBlockVector.push_back(location3);

}
