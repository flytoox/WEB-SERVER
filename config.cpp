/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obelaizi <obelaizi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/08 18:35:45 by obelaizi          #+#    #+#             */
/*   Updated: 2024/02/01 16:13:10 by obelaizi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "configFile.hpp"
#include "webserve.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <set>

using namespace std;

//TODO: Omar check index value if it has something; if not , set the value to index.html in the directives server block
//TODO: Omar check location match if it's duplicated , error -> exit
//TODO: Omar check the return if it gets Resonse StatusCode and next to it a URL; error -> exit
//TODO: Omar don't remove /// in location -> DONNNNNN'T
//DONE: Omar if autoindex is on , if autoindex is off remove autoindex key from scratch

//TODO: you need to put the value of directives 
//TODO :, then override them if they exist within that location block that you are parsing ; skip these three values :
//TODO: server_name, host, listen 

//TODO: set [upload_store] [/Users/sizgunan/goinfre/upload] -> if [upload_enable] == [on]



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
//TODO: cheange server_name to host
//TODO : ALERT CHANGE THE NAMES 


//TODO : if you find listen && port the same -> duplicated : true 
void adjustServerAddress(Server &server, struct sockaddr_in &serverAddress) {

    bzero(&serverAddress, sizeof(serverAddress));

    int port = atoi(((server.getdirectives().find("listen"))->second).c_str());
	server.prePort = (server.getdirectives().find("listen")->second);
    serverAddress.sin_family = AF_INET;
	string host= ((server.getdirectives().find("host"))->second);
	server.preHost = host;
	string ultimateHost = convertDomainToIPv4(host);
	if ( ultimateHost.empty()  ) {
		//TODO : throw expceptions
		cout << "Invalid\n"; exit (0);
	}
    //serverAddress.sin_addr.s_addr = inet_pton(AF_INET, ultimateHost.c_str(), &serverAddress.sin_addr);
	serverAddress.sin_addr.s_addr = inet_addr(ultimateHost.c_str());
    serverAddress.sin_port = htons(port);
}

void	duplicateServerBasedOnListen(vector<Server> &servers) {
	for (size_t i = 0; i < servers.size(); i++) {
			vector<string> listen = splitWithChar(servers[i].directives["listen"], ',');
			if (listen.size() == 1)
				continue;
			for (size_t j = 0; j < listen.size(); j++) {
				Server tmp = servers[i];
				tmp.directives["listen"] = listen[j];
				servers.push_back(tmp);
			}
			servers.erase(servers.begin() + i);
			i--;
	}
}

void alo() {
	return ;
}

static vector<string> split(string s) {
	stringstream ss(s);
	vector<string> v;
	string word;
	while (ss >> word)
		v.push_back(word);
	return (v);
}

vector<Server> parsingFile(string s) {
	stack<string> st;
	vector<Server> servers;
	Server server;
	map<string, string> directives;
	vector<map<string, string> > locationsBlock;
	string line;
	ifstream file(s);
	if (file.is_open())
	{
		while (getline(file, line))
		{
			vector<string> v = split(line);
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
				if (v.size() == 1) {
					if (!st.empty()) throw runtime_error("Can't have a block inside a block");
					st.push(v[0]);
					locationsBlock.clear();
					directives.clear();
					continue;
				}
				if (v.size() != 2)
					throw runtime_error("Error: too many arguments for " + v[0]);
				st.push(v[0]);
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
			directives[v[0]] = v[1];
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
		return (cout << "FUCK U DONT PLAY WITH ME\n", servers);
	duplicateServerBasedOnListen(servers);
	alo();
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
		// servers.erase(servers.begin()+1, servers.begin() + servers.size());
		// break ;
	}
	// set index.html if index is empty, and remove autoindex if it's off
	for (size_t i = 0; i < servers.size(); i++) {
		for (size_t j = 0; j < servers[i].locationsBlock.size(); j++) {
			// if (!servers[i].locationsBlock[j].count("index"))
			// 	servers[i].locationsBlock[j]["index"] = "index.html";
			if (servers[i].locationsBlock[j].count("autoindex") && servers[i].locationsBlock[j]["autoindex"] == "off")
				servers[i].locationsBlock[j].erase("autoindex");
		}
	}
	return (servers);
}

