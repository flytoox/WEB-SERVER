/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obelaizi <obelaizi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/08 18:35:45 by obelaizi          #+#    #+#             */
/*   Updated: 2023/12/18 10:31:38 by obelaizi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "configFile.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <set>

using namespace std;


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

vector<string> ft_split(string s, char c) {
	vector<string> v;
	string tmp;
	for (size_t i = 0; i < s.size(); i++) {
		if (s[i] == c) {
			v.push_back(tmp);
			tmp.clear();
		}
		else
			tmp += s[i];
	}
	if (tmp.size())
		v.push_back(tmp);
	return (v);
}
void	duplicateServerBasedOnListen(vector<Server> &servers) {
	for (size_t i = 0; i < servers.size(); i++) {
			vector<string> listen = ft_split(servers[i].directives["listen"], ',');
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

vector<Server> parsingFile(string s) {
	stack<string> st;
	vector<Server> servers;
	Server server;
	map<string, string> directives;
	map<string, string> serverDirectives;
	vector<map<string, string> > locationsBlock;
	string line;
	ifstream file(s);
	string WordWorking;
	if (file.is_open())
	{
		while (getline(file, line))
		{
			stringstream ss(line);
			string word, key="";
			while (ss >> word) {
				if (word == "location") {
					st.push(word);
					WordWorking = word;
					directives.clear();
					if (ss >> word)
						directives["location match"] = word;
					continue;
				}
				if (st.empty()) {
					st.push(word);
					WordWorking = word;
					continue;
				}
				if (word == "{") {
					st.push(word);
					continue;
				}
				if (word == "}") {
					if (!st.empty())
							st.pop();
					if (!st.empty() && st.top() == "server") {
						server.locationsBlock = locationsBlock;
						server.directives = serverDirectives;
						servers.push_back(server);
						server.directives.clear();
						server.locationsBlock.clear();
						directives.clear();
						serverDirectives.clear();
						locationsBlock.clear();
						if (!st.empty())
							st.pop();
					}
					if (!st.empty() && st.top() == "location") {
						locationsBlock.push_back(directives);
						directives.clear();
						if (!st.empty())
							st.pop();
					}
					continue;
				}
				if (WordWorking == "server") {
					GetDirectives(word, serverDirectives, key);
					continue;
				}
				if (WordWorking == "location") {
					GetDirectives(word, directives, key);
					continue;
				}
				cout << "ERROR MOTHER FUCKER\n";
				return servers;
			}
		}
		file.close();
	}
	if (!st.empty())
		return (cout << "FUCK U DONT PLAY WITH ME\n", servers);
	duplicateServerBasedOnListen(servers);
	set<pair<string, string>> Check;
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
	for (auto &i: servers) {
		std::cerr << "-->" << i.duplicated << endl;
	}
	return (servers);
}

