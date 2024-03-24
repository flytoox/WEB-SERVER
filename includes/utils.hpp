/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adnane <adnane@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/07 12:26:12 by aait-mal          #+#    #+#             */
/*   Updated: 2024/03/24 17:24:10 by adnane           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

#include "../includes/webserve.hpp"

std::vector<std::string> splitWithChar(std::string s, char delim);
std::vector<std::string> splitString(const std::string& input, const std::string& delimiter);
std::string ftToString(size_t n);
char hexToCharacters(const std::string& hex);
std::string decodeUrl(const std::string &srcString);
std::string lower(std::string str);

#endif
