#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void handle_cgi_get(const std::string& file, std::string& response) {
	try {
		Pipe	pipe;
		pid_t	pid = fork();

		if (pid == -1) {
			std::cerr << "Error forking process.\n";
			return;
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

			execve(phpPath.c_str(), argv, envp);

			// If execve fails
			std::cerr << "Error executing php: " << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}

		// Parent process
		else {
			close(pipe.getWriteEnd());

			// Read the output from the pipe into a stringstream
			std::ostringstream  ss;
			std::vector<char>   buffer(4096);
			ssize_t				bytesRead;

			while ((bytesRead = read(pipe.getReadEnd(), buffer.data(), buffer.size())) >= 0) {
				if (bytesRead == 0) break;  // End of file
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
		}
	} catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

bool handle_cgi_post(const std::string& file, const std::string& postData, std::string& response) {
    try {
        Pipe pipe;
        pid_t pid = fork();

        if (pid == -1) {
            std::cerr << "Error forking process.\n";
            return false;
        }

        // Child process
        if (pid == 0) {
            close(pipe.getReadEnd());

            // Redirect stdout and stderr to the pipe
            dup2(pipe.getWriteEnd(), STDOUT_FILENO);
            dup2(pipe.getWriteEnd(), STDERR_FILENO);

            // Set the CONTENT_LENGTH environment variable
            std::string contentLength = "CONTENT_LENGTH=" + std::to_string(postData.length());
            char* const envp[] = {const_cast<char*>(contentLength.c_str()), nullptr};

            // Construct the argument vector for execve
            std::string phpPath = "/usr/bin/php";
            char* const argv[] = {const_cast<char*>(phpPath.c_str()), const_cast<char*>(file.c_str()), nullptr};
            
            // Execute the CGI script
            execve(phpPath.c_str(), argv, envp);

            // If execve fails
            std::cerr << "Error executing php: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }

        // Parent process
        else {
            close(pipe.getWriteEnd());

            // Write the POST data to the child process
            write(pipe.getReadEnd(), postData.c_str(), postData.length());
            close(pipe.getReadEnd());  // Close the write end after writing

            // Read the output from the pipe into a stringstream
            std::ostringstream ss;
            std::vector<char> buffer(4096);
            ssize_t bytesRead;

            while ((bytesRead = read(pipe.getReadEnd(), buffer.data(), buffer.size())) >= 0) {
                if (bytesRead == 0) break;  // End of file
                ss.write(buffer.data(), bytesRead);
            }

            if (bytesRead == -1) {
                // Handle read error
                std::cerr << "Error reading from pipe: " << strerror(errno) << "\n";
                return false;
            }

            // Convert the stringstream to a string
            response = ss.str();

            // Wait for the child process to finish
            int status;
            waitpid(pid, &status, 0);

            // Check if the child process exited successfully
            return WIFEXITED(status) && WEXITSTATUS(status) == 0;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return false;
    }
}

