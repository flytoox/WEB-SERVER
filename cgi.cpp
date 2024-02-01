#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "webserve.hpp"

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

std::string handle_cgi_get(const std::string& file, const std::string& interpreterPath) {
    try {
		std::string response;
        Pipe pipe;
        pid_t pid = fork();

        if (pid == -1) {
            std::cerr << "Error forking process.\n";
            return std::string();
        }

        // Child process
        if (pid == 0) {
            close(pipe.getReadEnd());

            // Redirect stdout and stderr to the pipe
            dup2(pipe.getWriteEnd(), STDOUT_FILENO);
            dup2(pipe.getWriteEnd(), STDERR_FILENO);

            // Construct the argument vector for execve
            std::string phpPath = "/usr/bin/php";
            char* const argv[] = {const_cast<char*>(phpPath.c_str()), const_cast<char*>(file.c_str()), nullptr};
            char* const envp[] = {nullptr};  // Environment variables (none in this case)

            execve(interpreterPath.c_str(), argv, envp);

            // If execve fails
            std::cerr << "Error executing php: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }

        // Parent process
        else {
            close(pipe.getWriteEnd());

            // Read the output from the pipe into a stringstream
            std::ostringstream ss;
            std::vector<char> buffer(4096);
            ssize_t bytesRead;

            while ((bytesRead = read(pipe.getReadEnd(), buffer.data(), buffer.size())) > 0) {
                ss.write(buffer.data(), bytesRead);
            }

            if (bytesRead == -1) {
                // Handle read error
                std::cerr << "Error reading from pipe: " << strerror(errno) << "\n";
            }

            // Convert the stringstream to a string
            response = ss.str();

            // Wait for the child process to finish
            int status;
            waitpid(pid, &status, 0);

			return response;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
		return std::string();
    }
}

std::string handle_cgi_post(const std::map<std::string, std::string>& postData, const std::string& interpreter, const std::string& scriptFilePath) {
    try {
        std::string response;
        Pipe pipe;
        pid_t pid = fork();

        if (pid == -1) {
            std::cerr << "Error forking process.\n";
            return response;  // Return an empty string or handle the error accordingly
        }

        std::cout << "POST DATA: \n";
        for (const auto& entry : postData) {
            std::cout << entry.first << " = " << entry.second << "\n";
        }

        std::cout << "INTERPRETER: " << interpreter << "\n";
        std::cout << "SCRIPT FILE: " << scriptFilePath << "\n";

        // Child process
        if (pid == 0) {
            close(pipe.getReadEnd());

            // Redirect stdout and stderr to the pipe
            dup2(pipe.getWriteEnd(), STDOUT_FILENO);
            dup2(pipe.getWriteEnd(), STDERR_FILENO);

            // Construct the argument vector and environment variables for execve
			std::vector<char*> argv;
			std::vector<char*> envp;

			argv.push_back(const_cast<char*>(interpreter.c_str()));
			argv.push_back(const_cast<char*>(scriptFilePath.c_str()));
			argv.push_back(nullptr);

			for (const auto& entry : postData) {
				std::string envVar = entry.first + "=" + entry.second;
				envp.push_back(strdup(envVar.c_str()));  // Use strdup to duplicate the string
			}

			envp.push_back(nullptr);

			execve(interpreter.c_str(), argv.data(), envp.data());

			// Free the duplicated strings after execve
			for (char* str : envp) {
				free(str);
			}

            // If execve fails
            std::cerr << "Error executing interpreter: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }

        // Parent process
        else {
            close(pipe.getWriteEnd());

            // Read the output from the pipe into a stringstream
            std::ostringstream ss;
            std::vector<char> buffer(4096);
            ssize_t bytesRead;

            while ((bytesRead = read(pipe.getReadEnd(), buffer.data(), buffer.size())) > 0) {
                ss.write(buffer.data(), bytesRead);
            }

            if (bytesRead == -1) {
                // Handle read error
                std::cerr << "Error reading from pipe: " << strerror(errno) << "\n";
            }

            // Convert the stringstream to a string
            response = ss.str();

            // Wait for the child process to finish
            int status;
            waitpid(pid, &status, 0);

            return response;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return std::string();  // Return an empty string or handle the exception accordingly
    }
}
