#pragma once

#include "webserve.hpp"

class Pipe {
	private:
		int pipefd[2];

	public:
		Pipe();
		~Pipe();
		int getReadEnd();
		int getWriteEnd();
};

void redirectStdoutStderr(Pipe& pipe);
std::string readFromPipeAndClose(int readEnd);
int executeChildProcess(const std::string& interpreter, const std::string& scriptFilePath,
    	std::map<std::string, std::string>& envVars);
std::vector<std::string> splitWithString(const std::string& s, const std::string& delimiter);
std::pair<std::string, std::string> splitHeadersAndBody(const std::string& response);
void handleTimeout(int signal);
std::map<std::string, std::string> parseHeaders(const std::string& headers);
std::map<std::string, std::string> fillEnv(std::map<std::string, std::string>& headers);
