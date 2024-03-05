#include "../includes/cgi.hpp"

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
    std::map<std::string, std::string>& envVars) {
    std::vector<char*> argv;
    std::vector<char*> envp;

    argv.push_back(const_cast<char*>(interpreter.c_str()));
    argv.push_back(const_cast<char*>(scriptFilePath.c_str()));
    argv.push_back(nullptr);

    // for (const auto& entry : envVars) {
    //     std::string envVar = entry.first + "=" + entry.second;
    //     envp.push_back(strdup(envVar.c_str()));  // Use strdup to duplicate the string
    // }

    // for (const std::pair<std::string, std::string>& entry : envVars) {
    //     std::string envVar = entry.first + "=" + entry.second;
    //     envp.push_back(strdup(envVar.c_str()));
    // }

    for (std::map<std::string, std::string>::iterator it = envVars.begin(); it != envVars.end(); ++it) {
        std::string envVar = it->first + "=" + it->second;
        envp.push_back(strdup(envVar.c_str()));
    }

    envp.push_back(nullptr);

    execve(interpreter.c_str(), argv.data(), envp.data());

    // Free the duplicated strings after execve
    for (size_t i = 0; i < envp.size(); ++i) {
        free(envp[i]);
    }

    // If execve fails
    std::cerr << "Error executing interpreter: " << strerror(errno) << "\n";
    exit(EXIT_FAILURE);
}

std::vector<std::string> splitWithString(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> result;
    std::size_t start = 0;
    std::size_t found = s.find(delimiter);

    while (found != std::string::npos) {
        result.push_back(s.substr(start, found - start));
        start = found + delimiter.length();
        found = s.find(delimiter, start);
    }

    result.push_back(s.substr(start));

    return result;
}

std::pair<std::string, std::string> splitHeadersAndBody(const std::string& response) {
    size_t headerEndPos = response.find("\r\n\r\n");

    if (headerEndPos == std::string::npos) {
        // No header delimiter found, treat the entire response as headers
        return std::make_pair(std::string(), response);
    }

    std::string headers = response.substr(0, headerEndPos);
    std::string body = response.substr(headerEndPos + 4); // Skip "\r\n\r\n"

    return std::make_pair(headers, body);
}

