/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adnane <adnane@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/07 12:25:42 by aait-mal          #+#    #+#             */
/*   Updated: 2024/03/24 17:00:47 by adnane           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/utils.hpp"

std::vector<std::string> splitWithChar(std::string s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss (s);
	std::string item;

	while (getline (ss, item, delim)) {
		result.push_back (item);
	}

	return result;
}

std::string ftToString(size_t n) {
    std::string s;
    std::stringstream out;
    out << n;
    s = out.str();
    return s;
}

char hexToCharacters(const std::string& hex) {
    std::stringstream ss;
    ss << std::hex << hex;
    unsigned int c;
    ss >> c;
    return static_cast<char>(c);
}

std::string decodeUrl(const std::string &srcString) {
    std::string result;
    size_t length = srcString.size();

    for (size_t i = 0; i < length; ++i) {
        if (srcString[i] == '%' && i + 2 < length) {
            std::string hex = srcString.substr(i + 1, 2);
            result += hexToCharacters(hex);
            i += 2;
        } else if (srcString[i] == '+') {
            result += ' ';
        } else {
            result += srcString[i];
        }
    }
    return result;
}

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

std::string lower(std::string str) {
    std::string ret = "";
    for (std::string::iterator it = str.begin(); it != str.end(); it++) {
        ret += std::tolower(*it);
    }
    return ret;
}
