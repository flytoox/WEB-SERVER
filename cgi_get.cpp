#include "cgi.hpp"

void redirectStdoutStderr(Pipe& pipe) {
    dup2(pipe.getWriteEnd(), STDOUT_FILENO);
    dup2(pipe.getWriteEnd(), STDERR_FILENO);
}

std::string readFromPipeAndClose(int readEnd) {
    std::ostringstream ss;
    std::vector<char> buffer(4096);
    ssize_t bytesRead;

    while ((bytesRead = read(readEnd, buffer.data(), buffer.size())) > 0) {
        ss.write(buffer.data(), bytesRead);
    }

    close(readEnd);

    if (bytesRead == -1) {
        std::cerr << "Error reading from pipe: " << strerror(errno) << "\n";
    }

    return ss.str();
}

int executeChildProcess(const std::string& interpreter, const std::string& scriptFilePath,
    	const std::map<std::string, std::string>& envVars) {
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

    execve(interpreter.c_str(), argv.data(), envp.data());

    // Free the duplicated strings after execve
    for (char* str : envp) {
        free(str);
    }

    // If execve fails
    std::cerr << "Error executing interpreter: " << strerror(errno) << "\n";
    exit(EXIT_FAILURE);
}
