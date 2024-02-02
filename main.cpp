#include "webserve.hpp"
#include <stdexcept>
#include <vector>


void parseConfigFile(configFile &configurationServers) {
	try {
		std::vector<Server> servers = parsingFile("test.conf");
    	configurationServers.setTheVector(servers);
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}

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