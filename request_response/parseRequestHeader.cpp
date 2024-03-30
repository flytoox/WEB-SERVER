/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseRequestHeader.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obelaizi <obelaizi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/26 17:28:24 by obelaizi          #+#    #+#             */
/*   Updated: 2024/03/30 00:33:30 by obelaizi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserve.hpp"

bool parseFirstLine(std::string &s, Request &request) {
    std::vector<std::string> lines = splitWhiteSpaces(s);
    if (lines.size() != 3 || lines[2] != "HTTP/1.1" || lines[1][0] != '/') {
        return false;
    }
    for (size_t i = 0; i < lines[0].length(); i++) {
        if (!isalpha(lines[0][i]) && !isupper(lines[0][i]))
            return false;
    }
    parseUri(request, lines[1]);
    request.setHttpVerb(lines[0]);
    request.setHTTPVersion(lines[2]);
    return true;
}

bool parseDefaultLine(std::string &s, Request &request) {
    std::vector<std::string> v = splitWithChar(s, ':');
    if (v.size() < 2)
        return false;
    if (v[0][v[0].length() - 1] == ' ')
        return false;
    int l = -1;
    while (v[1][++l] == ' ');
    v[1].erase(0, l);
    if ((v[0] == "Content-Length" || v[0] == "Host" || v[0] == "Transfer-Encoding") && request.getHttpRequestHeaders().count(v[0]))
        return false;
    std::string value = "";
    for (size_t i = 1; i < v.size(); i++) {
        value += v[i];
        if (i < (s[s.length()-1] == ':' ? v.size() : v.size() - 1))
            value += ":";
    }
    request.setHttpRequestHeaders(make_pair(v[0], value.erase(value.length() - 2)));
    return true;
}
