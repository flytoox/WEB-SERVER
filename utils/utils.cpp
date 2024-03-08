/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aait-mal <aait-mal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/07 12:25:42 by aait-mal          #+#    #+#             */
/*   Updated: 2024/03/07 12:29:35 by aait-mal         ###   ########.fr       */
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