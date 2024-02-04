#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "../webserve.hpp"

class Pipe {
	public:
		Pipe() {
			if (pipe(pipefd) == -1) {
				std::cerr << "Error creating pipe.\n";
				throw std::runtime_error("Error creating pipe");
			}
		}

		~Pipe() {
			close(pipefd[0]);
			close(pipefd[1]);
		}

		int getReadEnd() const {
			return pipefd[0];
		}

		int getWriteEnd() const {
			return pipefd[1];
		}

	private:
		int pipefd[2];
};

void redirectStdoutStderr(Pipe& pipe);
std::string readFromPipeAndClose(int readEnd);
int executeChildProcess(const std::string& interpreter, const std::string& scriptFilePath,
    	const std::map<std::string, std::string>& envVars);
int executeChildProcessWithInput(const std::string& interpreter,
        const std::string& scriptFilePath,
        const std::map<std::string, std::string>& envVars,
        const std::string& inputData);
std::vector<std::string> splitWithString(const std::string& s, const std::string& delimiter);
std::pair<std::string, std::string> splitHeadersAndBody(const std::string& response);
