#include "includes/webserve.hpp"
#include <stdexcept>
#include <unistd.h>
#include <vector>


void parseConfigFile(configFile &configurationServers, std::string path) {
	try {
		std::vector<Server> servers = parsingFile(path);
    	configurationServers.setTheVector(servers);
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}

    funcMultiplexingBySelect(configurationServers);

}

void checkBasicErrors(int argc, std::string path) {
	if (argc != 2) {
		std::cerr << "Error: invalid number of arguments" << std::endl;
		exit(1);
	}
	if (path.size() < 5 || !path.compare(path.size() - 4, 4, ".conf")) {
		std::cerr << "Error: invalid file extension" << std::endl;
		exit(1);
	}
	if (access(path.c_str(), F_OK | R_OK) == -1) {
		std::cerr << "Error: file does not exist or is not readable" << std::endl;
		exit(1);
	}
} 

int main(int argc, char **argv) {

    configFile	configurationServers;

	checkBasicErrors(argc, argv[1]);
	parseConfigFile(configurationServers, argv[1]);

	return 0;
}