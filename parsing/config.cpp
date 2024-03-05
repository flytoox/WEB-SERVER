/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obelaizi <obelaizi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/08 18:35:45 by obelaizi          #+#    #+#             */
/*   Updated: 2024/03/04 17:03:23 by obelaizi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserve.hpp"
#include <stdexcept>


using namespace std;

//Done: Omar check location if it's duplicated , error -> exit
//Done: Omar check the return if it gets Resonse StatusCode and next to it a URL; error -> exit
//Done: Omar don't remove /// in location -> DONNNNNN'T
//DONE: Omar if autoindex is on , if autoindex is off remove autoindex key from scratch
vector<string> splitWhiteSpaces(string s) {
	stringstream ss(s);
	vector<string> v;
	string word;
	while (ss >> word)
		v.push_back(word);
	return (v);
}
bool checkReturnOnLocation(vector<map<string, string> > &locationsBlock) {
	for (size_t i = 0; i < locationsBlock.size(); i++) {
		if (!locationsBlock[i].count("return"))
			continue;
		std::string returnStr = locationsBlock[i]["return"];
		vector<string> v = splitWhiteSpaces(returnStr);
		if (v.size() != 2)
			return (false);
		if (v[0].size() > 3 || v[0].size() == 0)
			return (false);
		for (size_t i = 0; i < v[0].size(); i++)
			if (!isdigit(v[0][i]))
				return (false);
		int statusCode = atoi(v[0].c_str());
		if (statusCode < 100 || statusCode > 599)
			return (false);
	}
	return (true);
}

bool checkDuplicateLocation(vector<map<string, string> > &locationsBlock) {
	set<string> locationSet;
	for (size_t i = 0; i < locationsBlock.size(); i++) {
		if (locationSet.count(locationsBlock[i]["location"])) {
			return (true);
		}
		locationSet.insert(locationsBlock[i]["location"]);
	}
	return (false);
}

bool checkPortMaxMin(string port) {
	if (port.size() > 5 || port.size() == 0)
		return (false);
	for (size_t i = 0; i < port.size(); i++)
		if (!isdigit(port[i]))
			return (false);
	int portInt = atoi(port.c_str());
	if (portInt < 1025 || portInt > 65535)
		return (false);
	return (true);
}

void	GetDirectives(string &word, map<string, string> &directives, string &key) {
	if (key == "")
		key = word;
	else {
		if (word.back() == ';')
			word.pop_back();
		directives[key] = word;
	}

}

void fatal(string expression) {
    cerr << expression << endl;
    exit (1);
}

string convertDomainToIPv4(string &domain)
{
    struct addrinfo hints, *result, *p;
    char ipstr[INET_ADDRSTRLEN];
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(domain.c_str(), NULL, &hints, &result) != 0)
        return "";

    for (p = result; p != NULL; p = p->ai_next)
    {
        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = reinterpret_cast<struct sockaddr_in *>(p->ai_addr);
            inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
            freeaddrinfo(result);
            return ipstr;
        }
    }
    freeaddrinfo(result);
    return "";
}
void Server::overrideLocations(Server &s) {
	map<string, string> ServerDirectives = s.getdirectives();
	for (auto &i : s.locationsBlock)
		for (auto &j : ServerDirectives) 
			if (!i.count(j.first) && j.first != "listen" && j.first != "host")
				i[j.first] = j.second;
}

//if you find listen && port the same -> duplicated : true 
void adjustServerAddress(Server &server, struct sockaddr_in &serverAddress) {

    bzero(&serverAddress, sizeof(serverAddress));

    int port = atoi(((server.getdirectives().find("listen"))->second).c_str());
	server.prePort = (server.getdirectives().find("listen")->second);
    serverAddress.sin_family = AF_INET;
	string host= ((server.getdirectives().find("host"))->second);
	server.preHost = host;
	string ultimateHost = convertDomainToIPv4(host);
	if (ultimateHost.empty()) {
		//throw expceptions
		cout << "Invalid\n"; exit (0);
	}
    //serverAddress.sin_addr.s_addr = inet_pton(AF_INET, ultimateHost.c_str(), &serverAddress.sin_addr);
	serverAddress.sin_addr.s_addr = inet_addr(ultimateHost.c_str());
    serverAddress.sin_port = htons(port);
}


vector<Server> Server::parsingFile(string s) {
	stringstream lineNumStr;
	stack<string> st;
	vector<Server> servers;
	Server server;
	map<string, string> directives;
	vector<map<string, string> > locationsBlock;
	string line;
	ifstream file(s);
	int lineNum = 0;
	if (file.is_open()) {
		while (getline(file, line)) {
			lineNum++;
			lineNumStr << lineNum;
			vector<string> v = splitWhiteSpaces(line);
			if (v.size() == 0 || v[0][0] == '#')
				continue;
			if (v.size() == 1 && v[0].back() != '{' && v[0].back() != '}') {
				throw runtime_error("Syntax error");
				return servers;
			}
			if (v.back().back() == '{') {
				v.back().pop_back();
				if (v.back() == "") v.pop_back();
				if (v.empty()) throw runtime_error("Syntax error");
				if (v.size() == 1 && v[0] == "server") { // on server brackets
					if (!st.empty()) throw runtime_error("Can't have a block inside a block");
					st.push(v[0]);
					locationsBlock.clear();
					directives.clear();
					continue;
				}
				if (v.size() != 2)
					throw runtime_error("Error: wrong number of arguments on " + v[0] + " line " + lineNumStr.str() );
				if (st.empty())
					throw runtime_error("Error: The location on line "+ lineNumStr.str() +" block should be inside server Block");
				st.push(v[0]);// on location brackets
				if (server.directives.empty())
					server.directives = directives;
				directives.clear();
				directives[v[0]] = v[1];
				continue;
			}
			if (v.size() == 1 && v[0] == "}") {
				if (st.empty()) throw runtime_error("Error: } without {");
				if (st.top() == "server") {
					server.locationsBlock = locationsBlock;
					servers.push_back(server);
					directives.clear();
					locationsBlock.clear();
					server.directives.clear();
					server.locationsBlock.clear();
				} else if (st.top() == "location") {
					locationsBlock.push_back(directives);
					directives.clear();
				}
				st.pop();
				continue;
			}
			if (v[1].back() == ';') v[1].pop_back();
			if ((v[0] == "host" || v[0] == "listen" || v[0] == "server_name") && (st.top() == "location")) {
				throw runtime_error("Error: on line " + lineNumStr.str()  + " \"" +  v[0] + "\" can't be inside location block");
			}
			if ((v[0] == "cgi_bin" || v[0] == "error_page") && directives.count(v[0]) && st.top() == "location")
                directives[v[0]] += '\n' + v[1];
			else directives[v[0]] = v[1];
			for (size_t i = 2; i < v.size(); i++) {
				if (v[i].back() == ';')
					v[i].pop_back();
				if (v[i] == "") continue;
				directives[v[0]] += " " + v[i];
			}
		}
		file.close();
	}
	if (!st.empty())
		throw runtime_error("Error: { without }");
	try {
		for (size_t i = 0; i < servers.size(); i++) {
				if (!checkPortMaxMin(servers[i].directives["listen"]))
					throw runtime_error("Error: Invalid port number on server Num " + to_string(i+1));
				if (checkDuplicateLocation(servers[i].locationsBlock))
					throw runtime_error("There is a duplicate Location on server Num " + to_string(i+1));
				if (!checkReturnOnLocation(servers[i].locationsBlock))
					throw runtime_error("Error: Invalid return on server Num " + to_string(i+1));
				fillErrorPages(servers[i]);
		}
	} catch (runtime_error &e) {
		cerr << e.what() << endl;
		exit(1);
	}
	set<std::pair<string, string>> Check;
	for (size_t i = 0; i < servers.size(); i++)
	{
		for (size_t j = 0; j < servers.size(); j++) {
			if (i == j) continue;
			if (servers[i].directives["listen"] == servers[j].directives["listen"]
			 && servers[i].directives["host"] == servers[j].directives["host"])
			 servers[i].duplicated = true, servers[j].duplicated = true;
		}
		adjustServerAddress(servers[i], servers[i].serverAddress);
		servers[i].setServerAddress(servers[i].serverAddress);


		if (( servers[i].socketD = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
			fatal("Error: Fail to create a Socket for Server 1");
		}
		int add = 1;
		setsockopt(servers[i].socketD, SOL_SOCKET, SO_REUSEADDR, &add, sizeof(add));
		servers[i].setSocketDescriptor(servers[i].socketD);
		//* SAVE HISTORY ( HOST & PORT )
		if (!Check.count({servers[i].directives["listen"], servers[i].directives["host"]})) {
			servers[i].bindSockets();
			servers[i].listenToIncomingConxs();
			Check.insert({servers[i].directives["listen"], servers[i].directives["host"]});
		}
	}
	// set index.html if index is empty, and remove autoindex if it's off
	for (size_t i = 0; i < servers.size(); i++) {
		for (size_t j = 0; j < servers[i].directives.size(); j++) {
			if (servers[i].directives.count("autoindex") && servers[i].directives["autoindex"] == "off")
				servers[i].directives.erase("autoindex");
		}
		overrideLocations(servers[i]);
	}

	return (servers);
}