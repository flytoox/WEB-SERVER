#include "Server.hpp"
#include "configFile.hpp"



void    adjustServer2DirectivesMap(std::map<std::string, std::string> &serverDirectives) {

    serverDirectives["listen"] = "2222";
    serverDirectives["server_name"] = "localhost";
    serverDirectives["autoindex"] = "on";
    serverDirectives["root"] = "/";
    serverDirectives["client_max_body_size"] = "2000000";
    serverDirectives["index"] = "index.html";

}


void    adjustServer2LocationBlockMap1(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/";
    serverLocationDirectives["root"] = "/Users/sizgunan";
    serverLocationDirectives["autoindex"] = "on";
    // serverLocationDirectives["return"] = "Server2 Success";

}

void    adjustServer2LocationBlockMap2(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/Documents";
    serverLocationDirectives["special"] = "DETECTED";
    serverLocationDirectives["root"] = "/Users/sizgunan";
    serverLocationDirectives["autoindex"] = "on";
    // serverLocationDirectives["return"] = "Server2 : Location : /Documents";

}

void    adjustServer2LocationBlockMap3(std::map<std::string, std::string> &serverLocationDirectives) {

    serverLocationDirectives["location match"] = "/Desktop";
    serverLocationDirectives["root"] = "/Users/sizgunan";
    serverLocationDirectives["autoindex"] = "on";
    // serverLocationDirectives["return"] = "Server2 : Location : /Desktop";
}


void adjustServer2LocationBlockVector(std::vector<std::map<std::string, std::string> > &server2LocationBlockVector) {


    std::map<std::string, std::string> location1;
    std::map<std::string, std::string> location2;
    std::map<std::string, std::string> location3;

    adjustServer2LocationBlockMap1(location1);
    adjustServer2LocationBlockMap2(location2);
    adjustServer2LocationBlockMap3(location3);


    server2LocationBlockVector.push_back(location1);
    server2LocationBlockVector.push_back(location2);
    server2LocationBlockVector.push_back(location3);

}
