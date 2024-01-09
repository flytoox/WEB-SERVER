#include "webserve.hpp"
#include <unistd.h>
#include <sstream>

// This function is called in GET method when a CGI script is requested.
void handle_cgi_get(const std::string& file, std::string& response) {
    // Create a pipe for IPC
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Error creating pipe.\n";
        return;
    }

    // Fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Error forking process.\n";
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }
    (void)file; // Avoid unused variable warning
    if (pid == 0) { // Child process
        // Close the read end of the pipe
        close(pipefd[0]);

        // Redirect stdout and stderr to the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        // Construct the argument vector for execve
        const char* phpPath = "/usr/bin/php";  // Adjust the path to your PHP interpreter
        const char* phpScript = "/Users/aait-mal/Desktop/example.php";  // Adjust the path to your PHP script

        char* const argv[] = {const_cast<char*>(phpPath), const_cast<char*>(phpScript), nullptr};
        char* const envp[] = {nullptr};  // Environment variables (none in this case)

        execve(phpPath, argv, envp);

        // If execve fails
        std::cerr << "Error executing php: " << strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Close the write end of the pipe
        close(pipefd[1]);

        // Read the output from the pipe into a stringstream
        std::ostringstream ss;
        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            ss.write(buffer, bytesRead);
        }

        // Convert the stringstream to a string
        response = ss.str();

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        // Close the remaining end of the pipe
        close(pipefd[0]);
    }
}

// This function is called in POST method when a CGI script is requested.
bool handle_cgi_post(const std::string& file, const std::string& body, std::string& response) {
    // Create a pipe for IPC
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Error creating pipe.\n";
        return false;
    }

    // Fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Error forking process.\n";
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    }
    (void)file; // Avoid unused variable warning
    if (pid == 0) { // Child process
        // Close the read end of the pipe
        close(pipefd[0]);

        // Redirect stdout and stderr to the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        // Construct the argument vector for execve
        const char* phpPath = "/usr/bin/php";  // Adjust the path to your PHP interpreter
        const char* phpScript = "/Users/aait-mal/Desktop/example.php";  // Adjust the path to your PHP script

        char* const argv[] = {const_cast<char*>(phpPath), const_cast<char*>(phpScript), nullptr};
        char* const envp[] = {nullptr};  // Environment variables (none in this case)

        execve(phpPath, argv, envp);

        // If execve fails
        std::cerr << "Error executing php: " << strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Close the write end of the pipe
        close(pipefd[1]);

        // Write the body to the pipe
        write(pipefd[0], body.c_str(), body.size());

        // Read the output from the pipe into a stringstream
        std::ostringstream ss;
        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            ss.write(buffer, bytesRead);
        }

        // Convert the stringstream to a string
        response = ss.str();

        std::cerr << "response: " << response << std::endl;

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        // Close the remaining end of the pipe
        close(pipefd[0]);

        return true;
    }
}
