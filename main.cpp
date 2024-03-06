#include "includes/webserve.hpp"

void parseConfigFile(configFile &configurationServers, std::string path) {
	try {
		std::vector<Server> servers = Server::parsingFile(path);
		if (servers.empty()) {
			throw std::runtime_error("Error: file has no server in it");
		}
    	configurationServers.setTheVector(servers);
    	funcMultiplexingBySelect(configurationServers);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

void checkBasicErrors(std::string path) {
	if (path.size() < 5 || strcmp(path.c_str() + path.size() - 5, ".conf")) {
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
	if (argc != 2) {
		std::cerr << "Error: invalid number of arguments" << std::endl;
		exit(1);
	}
	signal(SIGPIPE, SIG_IGN);
	checkBasicErrors(argv[1]);
	parseConfigFile(configurationServers, argv[1]);
	return 0;
}
