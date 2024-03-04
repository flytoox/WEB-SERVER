/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obelaizi <obelaizi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/04 15:30:31 by obelaizi          #+#    #+#             */
/*   Updated: 2024/03/04 17:39:22 by obelaizi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserve.hpp"

bool StringIsNum(std::string &s) {
    for (char &c:s) if (!isdigit(c)) return (false);
    if (s.size() > 3 || s.size() == 0) return (false);
    return (true);
}

void fillTheMap(std::map<int, std::string> &map, std::string &Err) {
    std::vector<std::string> v = splitWithChar(Err, '\n');
    int num;
    for (std::string &line : v) {
        std::vector<std::string> v2 = splitWithChar(line, ' ');
        if (v2.size() != 2)
            throw std::runtime_error("Error: error_page has invalid format");
        StringIsNum(v2[0]) == false ? throw std::runtime_error("Error: invalid error code") : num = atoi(v2[0].c_str());
        if (num < 100 || num > 599)
            throw std::runtime_error("Error: error code should be between 100 and 599");
        std::ifstream file(v2[1]);
        if (!file) continue;
        std::string s = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        if (s.size() > 1e6) continue;
        map[num] = s;
    }
    std::vector<int> statusCodes = {200, 201, 204, 301, 400, 403, 404, 405, 409, 413, 414, 500, 501, 502};
    for (int &i : statusCodes) {
        std::stringstream codeStr;
        codeStr << i;
        if (map.count(i) == 0) {
            std::ifstream file("./response_pages/" + codeStr.str() + ".html");
            if (!file) throw std::runtime_error("Error: missing error page " + codeStr.str() + ".html");
            std::string s = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            if (s.size() > 1e6) throw std::runtime_error("Error: error page " + codeStr.str() + ".html is too big");
            map[i] = s;
        }
    }
}

void Server::fillErrorPages(Server &s) {
    std::string ErrorPages;
    for (auto &i : s.locationsBlock) {
        ErrorPages = i["error_page"];
        try {
            fillTheMap(s.pages[i["location"]], ErrorPages);
        } catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }
    }
}