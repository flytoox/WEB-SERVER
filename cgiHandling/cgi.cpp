#include "cgi.hpp"

void handleTimeout(int signal) {
    // Handle timeout signal
    if (signal == SIGALRM) {
        std::cerr << "Child process timed out and is being terminated.\n";
        _exit(EXIT_FAILURE);
    }
}

std::map<std::string, std::string> parseHeaders(const std::string& headers) {
    std::map<std::string, std::string> headerMap;

    // Split the headers into lines
    std::istringstream headerStream(headers);
    std::string line;
    getline(headerStream, line);  // Read the first line separately

    // Set the first line as "HTTP_REQUEST" key with the value of the entire line
    headerMap["HTTP_REQUEST"] = line;

    // Process the rest of the lines
    while (getline(headerStream, line)) {
        // Find the position of the colon (':')
        size_t colonPos = line.find(':');

        // If a colon is found, extract the key and value
        if (colonPos != std::string::npos) {
            // Extract the key and value
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2);  // Skip the space after the colon

            // Standardize and uppercase the key
            for (size_t i = 0; i < key.length(); ++i) {
                key[i] = toupper(key[i]);
                if (key[i] == '-')
                    key[i] = '_';
            }

            // Add "HTTP_" prefix to the key except for specific headers
            if (key != "HTTP_USER_AGENT" && key != "HTTP_REFERER"
                && key != "CONTENT_LENGTH" && key != "CONTENT_TYPE"
                && key != "AUTH_TYPE" && key != "REMOTE_USER"
                && key != "REMOTE_IDENT") {
                key = "HTTP_" + key;
            }

            headerMap[key] = value;
        }
    }

    return headerMap;
}

std::map<std::string, std::string> fillEnv(const std::map<std::string, std::string>& headers) {
    std::map<std::string, std::string> envVars;

    // Set up the environment variables
    envVars["SERVER_SOFTWARE"] = "webserve";
    envVars["SERVER_NAME"] = "localhost";
    envVars["GATEWAY_INTERFACE"] = "CGI/1.1";
    envVars["SERVER_PROTOCOL"] = "HTTP/1.1";

    envVars["REDIRECT_STATUS"] = "1";

    for (const auto& entry : headers) {
        if (entry.first == "HTTP_REQUEST") {
            // Parse the HTTP_REQUEST header to extract relevant information
            std::istringstream requestStream(entry.second);
            std::vector<std::string> requestTokens;
            std::string token;

            // Tokenize the HTTP_REQUEST header
            while (requestStream >> token) {
                requestTokens.push_back(token);
            }

            // Set REQUEST_METHOD
            if (!requestTokens.empty()) {
                envVars["REQUEST_METHOD"] = requestTokens[0];
            }

            // Set REQUEST_URI and parse QUERY_STRING
            if (requestTokens.size() > 1) {
                envVars["REQUEST_URI"] = requestTokens[1];

                // Parse QUERY_STRING if it exists
                size_t queryStringPos = envVars["REQUEST_URI"].find('?');
                if (queryStringPos != std::string::npos) {
                    envVars["QUERY_STRING"] = envVars["REQUEST_URI"].substr(queryStringPos + 1);
                }
            }

            // Set SERVER_PROTOCOL
            if (requestTokens.size() > 2) {
                envVars["SERVER_PROTOCOL"] = requestTokens[2];
            }
        } else {
            envVars[entry.first] = entry.second;
        }
    }

    return envVars;
}


std::pair<std::string, std::string> handle_cgi_get(const std::string& file,
                                        const std::string& interpreterPath,
                                        Request &request) {
    try {
        std::string response;
        Pipe pipe;
        pid_t pid;


        std::string Header = request.getRequestHeader();
        std::map<std::string, std::string> headers = parseHeaders(Header);
        std::map<std::string, std::string> envVars;
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

std::pair<std::string, std::string> handle_cgi_post(const std::string& file,
                                        const std::string& interpreterPath,
                                        Request &request) {
    try {
        std::string response;
        Pipe pipe;
        pid_t pid;

        std::string Header = request.getRequestHeader();
        std::map<std::string, std::string> headers = parseHeaders(Header);
        std::map<std::string, std::string> envVars;
        pid = fork();

        if (pid == -1) {
            std::cerr << "Error forking process.\n";
            return std::make_pair(std::string(), std::string());
        }

        if (pid == 0) {  // Child process
            close(pipe.getReadEnd());
            redirectStdoutStderr(pipe);

            signal(SIGALRM, handleTimeout);
            alarm(10); // Timeout after 10 seconds

            envVars = fillEnv(headers);

            envVars["SCRIPT_NAME"] = file;
            envVars["SCRIPT_FILENAME"] = file;

            std::string postData = request.getRequestBody();
            std::cout << "POST DATA: " << postData << "\n";

            // Set up environment variables
            envVars["CONTENT_LENGTH"] = std::to_string(postData.length());
            envVars["CONTENT_TYPE"] = extractContentType(Header);

            // Execute the child process with input data
            executeChildProcessWithInput(interpreterPath, file, envVars, postData);

            alarm(0);
        }
        else {  // Parent process
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

int executeChildProcessWithInput(const std::string& interpreter,
                                 const std::string& scriptFilePath,
                                 const std::map<std::string, std::string>& envVars,
                                 const std::string& inputData) {
    std::vector<char*> argv;
    std::vector<char*> envp;

    argv.push_back(const_cast<char*>(interpreter.c_str()));
    argv.push_back(const_cast<char*>(scriptFilePath.c_str()));
    argv.push_back(nullptr);

    for (const auto& entry : envVars) {
        std::string envVar = entry.first + "=" + entry.second;
        envp.push_back(strdup(envVar.c_str()));  // Use strdup to duplicate the string
    }

    envp.push_back(nullptr);

    // Redirect standard input to read from a pipe
    dup2(STDIN_FILENO, STDIN_FILENO);

    // Write inputData to the pipe (standard input of the child)
    write(STDIN_FILENO, inputData.c_str(), inputData.length());
    close(STDIN_FILENO);  // Close the standard input after writing

    // Execute the child process
    execve(interpreter.c_str(), argv.data(), envp.data());

    // If execve fails
    std::cerr << "Error executing interpreter: " << strerror(errno) << "\n";
    exit(EXIT_FAILURE);
}

