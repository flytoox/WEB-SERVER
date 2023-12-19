#include "webserve.hpp"


// void fatal(std::string expression) {
//     std::cerr << expression << std::endl;
//     exit (1);
// }


// void adjustServerAddress(Server &server, struct sockaddr_in &serverAddress) {

//     bzero(&serverAddress, sizeof(serverAddress));

//     int port = std::atoi(((server.getdirectives().find("listen"))->second).c_str());

//     serverAddress.sin_family = AF_INET;
//     serverAddress.sin_addr.s_addr = htonl(2130706433);
//     serverAddress.sin_port = htons(port);
// }

void parseConfigFile(configFile &configurationServers) {


    // //*        SERVER1

    // Server server1;
    // std::map<std::string, std::string> server1DirectivesMap;
    // std::vector<std::map<std::string, std::string> > server1LocationBlockVector;
    // struct sockaddr_in serverAddress1;
    // int socketDes1;

    // adjustServer1DirectivesMap(server1DirectivesMap);
    // adjustServer1LocationBlockVector(server1LocationBlockVector);
    // server1.setDirectives(server1DirectivesMap);
    // server1.setLocationBlock(server1LocationBlockVector);

    // adjustServerAddress(server1, serverAddress1);
    // server1.setServerAddress(serverAddress1);
    


    // if (( socketDes1 = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
    //     fatal("Error: Fail to create a Socket for Server 1");
    // }
    // int add = 1;
    // setsockopt(socketDes1, SOL_SOCKET, SO_REUSEADDR, &add, sizeof(add));

    // server1.setSocketDescriptor(socketDes1);
    // server1.bindSockets();
    // server1.listenToIncomingConxs();



    
    // //*        SERVER2

    // Server server2 ;
    // std::map<std::string, std::string> server2DirectivesMap;
    // std::vector<std::map<std::string, std::string> > server2LocationBlockVector;
    // struct sockaddr_in serverAddress2;
    // int socketDes2;

    // adjustServer2DirectivesMap(server2DirectivesMap);
    // adjustServer2LocationBlockVector(server2LocationBlockVector);
    // server2.setDirectives(server2DirectivesMap);
    // server2.setLocationBlock(server2LocationBlockVector);


    // adjustServerAddress(server2, serverAddress2);
    // server2.setServerAddress(serverAddress2);
        

    // if (( socketDes2 = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
    //     fatal("Error: Fail to create a Socket for Server 2");
    // }

    // setsockopt(socketDes2, SOL_SOCKET, SO_REUSEADDR, &add, sizeof(add));

    // server2.setSocketDescriptor(socketDes2);
    // server2.bindSockets();
    // server2.listenToIncomingConxs();    

    // //*        SERVER3

    // Server server3;
    // std::map<std::string, std::string> server3DirectivesMap;
    // std::vector<std::map<std::string, std::string>  > server3LocationBlockVector;
    // struct sockaddr_in serverAddress3;
    // int socketDes3;

    // adjustServer3DirectivesMap(server3DirectivesMap);
    // adjustServer3LocationBlockVector(server3LocationBlockVector);
    // server3.setDirectives(server3DirectivesMap);
    // server3.setLocationBlock(server3LocationBlockVector);

    // adjustServerAddress(server3, serverAddress3);
    // server3.setServerAddress(serverAddress3);

    // if (( socketDes3 = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
    //     fatal("Error: Fail to create a Socket for Server 3");
    // }

    // setsockopt(socketDes3, SOL_SOCKET, SO_REUSEADDR, &add, sizeof(add));

    // server3.setSocketDescriptor(socketDes3);
    // server3.bindSockets();  
    // server3.listenToIncomingConxs();

    // configurationServers.setServers(server1);
    // configurationServers.setServers(server2);
    // configurationServers.setServers(server3);
    configurationServers.setTheVector(parsingFile("test.conf"));

    funcMultiplexingBySelect(configurationServers);

}

int main(void) {


    configFile configurationServers;

    // try {
    
        parseConfigFile(configurationServers);

    // } catch (std::bad_exception &e) {
    //     std::cout << e.what() << std::endl;
    // }


}