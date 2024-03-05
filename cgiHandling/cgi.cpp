#include "../includes/cgi.hpp"

std::pair<std::string, std::string> handleCgiGet(const std::string& file,
                                        const std::string& interpreterPath,
                                        Request &request) {
    try {
        std::string response;
        Pipe pipe;
        pid_t pid;


        std::string Header = request.getRequestHeader();

        std::map<std::string, std::string> headers = parseHeaders(Header);
        std::map<std::string, std::string> envVars;

        // std::string checkbody = request.getRequestBody();
        // // erase the whole body of request
        // request.setRequestBody(std::string());
        // checkbody = request.getRequestBody();
        // std::cout<< "CECKBODY <<<<<<<<<<" << checkbody << std::endl;
        // envVars["REQUEST_METHOD"] = "GET";
        // if (checkbody.length() > 0) {
        //     return std::make_pair("Content-Type: text/html\r\n", "<html><h1>405 Method Not Allowed</h1></html>");
        // }

        pid = fork();

        if (pid == -1) {
            std::cerr << "Error forking process.\n";
            return std::make_pair(std::string(), std::string());
        }

        if (pid == 0) {  // Child process
            close(pipe.getReadEnd());
            redirectStdoutStderr(pipe);

            // Set up the environment variables
            envVars = fillEnv(headers);
            //find CONTENT_LENGTH and CONTENT_TYPE and delete them
            envVars.erase("CONTENT_LENGTH");
            envVars.erase("CONTENT_TYPE");

            envVars["SCRIPT_NAME"] = file;
            envVars["SCRIPT_FILENAME"] = file;

            // Set up a timeout using the alarm function
            signal(SIGALRM, handleTimeout);
            alarm(10); // Timeout after 10 seconds

            executeChildProcess(interpreterPath, file, envVars);

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
                
                return std::make_pair("Content-Type: text/html\r\n", request.getPageStatus(500));
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

std::pair<std::string, std::string> handleCgiPost(const std::string& file,
                                        const std::string& interpreterPath,
                                        Request &request) {
    try {
        std::string response;
        Pipe pipe;
        pid_t pid;

        std::string Header = request.getRequestHeader();

        std::map<std::string, std::string> mapHeaders = request.getHttpRequestHeaders();
        std::map<std::string, std::string> headers = parseHeaders(Header);
        std::map<std::string, std::string> envVars;

        std::string postData = request.getRequestBody();
        std::stringstream ss;
        ss << postData.length();

        //write post data to a tmp file
        std::ofstream tmpFile("tmpFile");
        tmpFile << postData;
        tmpFile.close();

        pid = fork();

        if (pid == -1) {
            std::cerr << "Error forking process.\n";
            return std::make_pair(std::string(), std::string());
        }

        if (pid == 0) {  // Child process
            close(pipe.getReadEnd());
            redirectStdoutStderr(pipe);

            // Redirect standard input to read from the temporary file
            int tmpFileDescriptor = open("tmpFile", O_RDONLY);
            if (tmpFileDescriptor < 0) {
                std::cerr << "Error opening tmpFile for reading.\n";
                exit(EXIT_FAILURE);
            }
            if (dup2(tmpFileDescriptor, STDIN_FILENO) == -1) {
                std::cerr << "Error redirecting standard input.\n";
                exit(EXIT_FAILURE);
            }
            close(tmpFileDescriptor);
            //delete the tmp file
            remove("tmpFile");

            // Set up a timeout using the alarm function
            signal(SIGALRM, handleTimeout);
            alarm(10); // Timeout after 10 seconds

            // Set up the environment variables
            envVars = fillEnv(headers);
            envVars["SCRIPT_NAME"] = file;
            envVars["SCRIPT_FILENAME"] = file;
            envVars["REQUEST_METHOD"] = "POST";
            envVars["CONTENT_TYPE"] = mapHeaders["Content-Type:"];
            envVars["CONTENT_LENGTH"] = std::to_string(postData.length());
            envVars["QUERY_STRING"] = request.getQueryString();

            executeChildProcess(interpreterPath, file, envVars);

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
                return std::make_pair("Content-Type: text/html\r\n", request.getPageStatus(500));
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
