#include "cgi.hpp"

void handleTimeout(int signal) {
    // Handle timeout signal
    if (signal == SIGALRM) {
        std::cerr << "Child process timed out and is being terminated.\n";
        _exit(EXIT_FAILURE);
    }
}

std::pair<std::string, std::string> handle_cgi_get(const std::string& file,
                                        const std::string& interpreterPath,
                                        Request &request) {
    try {
        std::string response;
        Pipe pipe;
        std::string Header = request.getRequestHeader();
        std::map<std::string, std::string> envVars;
        std::cout << "Header: " << Header << std::endl;
        pid_t pid = fork();



        if (pid == -1) {
            std::cerr << "Error forking process.\n";
            return std::make_pair(std::string(), std::string());
        }

        if (pid == 0) {  // Child process
            close(pipe.getReadEnd());
            redirectStdoutStderr(pipe);


            // Set up a timeout using the alarm function
            signal(SIGALRM, handleTimeout);
            alarm(10); // Timeout after 10 seconds

            executeChildProcess(interpreterPath, file, {});

            // Reset the alarm if the child process finishes before the timeout
            alarm(0);
        } else {  // Parent process
            close(pipe.getWriteEnd());
            response = readFromPipeAndClose(pipe.getReadEnd());
            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                int exitStatus = WEXITSTATUS(status);
                std::cout << "Child process exited with status: " << exitStatus << "\n";
            } else if (WIFSIGNALED(status)) {
                int signalNumber = WTERMSIG(status);
                std::cerr << "Child process terminated by signal: " << signalNumber << "\n";
                return std::make_pair(std::string(), "<html><h1>500 Internal Server Error</h1></html>");
            } else {
                std::cerr << "Child process terminated abnormally.\n";
            }
        }

        return splitHeadersAndBody(response);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return std::make_pair(std::string(), std::string());
    }
}

std::pair<std::string, std::string> handle_cgi_post(const std::map<std::string, std::string>& postData,
                                        const std::string& interpreter, const std::string& scriptFilePath) {
    try {
        std::string response;
        Pipe pipe;
        pid_t pid = fork();

        if (pid == -1) {
            std::cerr << "Error forking process.\n";
            return std::make_pair(std::string(), std::string());
        }

        if (pid == 0) {  // Child process
            close(pipe.getReadEnd());
            redirectStdoutStderr(pipe);

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
        } else {  // Parent process
            close(pipe.getWriteEnd());
            response = readFromPipeAndClose(pipe.getReadEnd());

            // Handle timeout if needed
            // ...

            int status;
            waitpid(pid, &status, 0);

            // Handle child process termination
            if (WIFEXITED(status)) {
                int exitStatus = WEXITSTATUS(status);
                std::cout << "Child process exited with status: " << exitStatus << "\n";
            } else if (WIFSIGNALED(status)) {
                int signalNumber = WTERMSIG(status);
                std::cerr << "Child process terminated by signal: " << signalNumber << "\n";
                // Handle termination by signal
            } else {
                std::cerr << "Child process terminated abnormally.\n";
                // Handle abnormal termination
            }

            return splitHeadersAndBody(response);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return std::make_pair(std::string(), std::string());
    }
}
