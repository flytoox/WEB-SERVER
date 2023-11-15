// #include "configurationFile.hpp"
// #include "webserv.hpp"
// #include <ostream>
// #include <sys/socket.h>


// // int main(int ac, char **av) {
// //     ConfigFile *confFile;
// //     if (ac != 2) {
// //         std::cout << "Invalid arguments" << std::endl;
// //         return 1;
// //     }
// //     std::ifstream ConfigFileFD (av[1]);
// //     confFile->setConfigFileFd(ConfigFileFD);
// //     if ( ! confFile->openCheckFile(ConfigFileFD) ) {
// //         std::cout << "Error: open Config File " << std::endl;
// //         return 1;
// //     }
// //     confFile->readConfigFile(ConfigFileFD);
// // }



// void parseConfigFile(ConfigFile &configFile) 
// {

//     std::vector<int> mainContextServers;

//     configFile.fillmainContextServersVector(mainContextServers);
//     int i = -1;

//     for (std::vector<int>::iterator it = mainContextServers.begin(); it != mainContextServers.end(); it++) {
//         ++i;
        

//         configFile.setDirectivesForEach(mapServer, i);
//     }

// }


// //DONE: Create the socket
// //TODO: Identify the socket
// //TODO: On the server, wait for a connection
// //TODO: Send and receive messages
// //TODO: Close the sockets

// void fatal(std::string error) {
//     std::cerr << error << std::endl;
//     exit (1);
// }

// typedef typename std::vector<int>::const_iterator vectorIterator;

// void handleSockets(ConfigFile &configFileObj) {

//     std::vector<int> vec = configFileObj.getmainContextServers();
//     for (vectorIterator it = vec.begin() ; it != vec.end(); it++) {
//         if (int socket_fd = socket(AF_INET, SOCK_STREAM, 0) < 0)
//             fatal("Error: fail to open a socket");

//     }
// }


// int main(void) {


//     ConfigFile configFileObj;



//     parseConfigFile(configFileObj);
//     handleSockets(configFileObj);

// }
