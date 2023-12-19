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

void fatal(std::string expression) {
    std::cerr << expression << std::endl;
    exit (1);
}


void adjustServerAddress(Server &server, struct sockaddr_in &serverAddress) {

    bzero(&serverAddress, sizeof(serverAddress));

    int port = std::atoi(((server.getdirectives().find("listen"))->second).c_str());

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(2130706433);
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
	for (size_t i = 0; i < servers.size(); i++)
	{
		cout << "Server " << i << endl;
		adjustServerAddress(servers[i], servers[i].serverAddress);
		servers[i].setServerAddress(servers[i].serverAddress);
		


		if (( servers[i].socketD = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
			fatal("Error: Fail to create a Socket for Server 1");
		}
		int add = 1;
		setsockopt(servers[i].socketD, SOL_SOCKET, SO_REUSEADDR, &add, sizeof(add));

		servers[i].setSocketDescriptor(servers[i].socketD);
		servers[i].bindSockets();
		servers[i].listenToIncomingConxs();
		for (map<string, string>::iterator it = servers[i].directives.begin(); it != servers[i].directives.end(); it++)
			cout << it->first << " = " << it->second << endl;
		for (size_t j = 0; j < servers[i].locationsBlock.size(); j++)
		{
			cout << "Location " << j << endl;
			for (map<string, string>::iterator it = servers[i].locationsBlock[j].begin(); it != servers[i].locationsBlock[j].end(); it++)
				cout << it->first << " = " << it->second << endl;
			cout << endl;
		}
		cout << endl << endl;
	}
	return (servers);
}

