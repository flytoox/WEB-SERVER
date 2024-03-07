/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aait-mal <aait-mal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/07 12:26:12 by aait-mal          #+#    #+#             */
/*   Updated: 2024/03/07 12:29:49 by aait-mal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

#include "../includes/webserve.hpp"

std::vector<std::string> splitWithChar(std::string s, char delim);
std::string ftToString(size_t n);
char hexToCharacters(const std::string& hex);
std::string decodeUrl(const std::string &srcString);

#endif