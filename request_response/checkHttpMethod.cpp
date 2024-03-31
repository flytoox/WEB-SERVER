/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checkHttpMethod.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obelaizi <obelaizi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 01:41:16 by obelaizi          #+#    #+#             */
/*   Updated: 2024/03/31 01:41:17 by obelaizi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserve.hpp"

void checkRequestedHttpMethod(Request &request) {
    std::string httpMethod = request.getHttpVerb();
    if (httpMethod == "GET")
        method(request, &getFile, &getFolder);
    else if (httpMethod == "POST")
        method(request, &postFile, &postFolder);
    else
        method(request, &deleteFile, &deleteFolder);
}
