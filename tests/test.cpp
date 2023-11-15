#include <string>
#include <iostream>
#include <stdlib.h>
#include <vector>


std::vector<std::string> splitString(const std::string& input, const std::string& delimiter) {
    std::vector<std::string> result;
    std::size_t start = 0;
    std::size_t end = input.find(delimiter);

    while (end != std::string::npos) {
        result.push_back(input.substr(start, end - start));
        start = end + delimiter.length();
        end = input.find(delimiter, start);
    }

    result.push_back(input.substr(start));

    return result;
}

int main(void) {

    std::string requestBody = "7\r\nMozilla\r\n11\r\nDeveloper Network\r\n";
    std::vector<std::string> res = splitString(requestBody, "\r\n");

    for (auto i : res)
        std::cout << "|" << i << "|" << std::endl;
    return (0);
}

