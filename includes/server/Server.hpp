/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 19:15:28 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/16 04:51:33 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <map>
# include <fcntl.h>
# include <cstring>
# include <poll.h>
# include "ListeningSocket.hpp"
# include "Client.hpp"
# include "../config-parser/ConfigParser.hpp"
# include "../http/HttpResponse.hpp"
# define SESSION_ID_BYTES 16
# define SESSION_TTL_SECONDS 1800

enum E_SOCKTYPE
{
	T_SERVER,
	T_CLIENT
};

struct Session 
{
	std::string	id;
	time_t		expiresAt;
	size_t		visitCount;
	std::string	name;
};

class Server
{
private:
	Config														_conf;
	std::vector<ListeningSocket>								_socks;
	std::map<int, Client>										_clients;
	std::vector<std::pair<int, std::pair<std::string, int> > >	_allListens;
	int															_serverCount;
	size_t														_listeningSocketsCount;
	std::map<std::string, Session>								_sessions;
public:
	Server(Config &config, std::vector<std::pair<int, std::pair<std::string, int> > > allListens, int serverCount);
	void														startServer();
private:
	void														openListeningSocks();
	void														handleSock(int sockfd, std::pair<std::string, int> bindInfo, E_SOCKTYPE type);
	int															setRUASock(int sockfd);
	int															setNBSock(int sockfd);
	void														bindAndListen(int sockfd, std::pair<std::string, int> bindInfo);
	void														timeoutChecker();
	void														acceptNewClient(ListeningSocket &acceptingSocket);
	void														deleteClient(int clientFd);
	void														handleClientInput(Client &client);
	void														handleClientOutput(Client &client);
	std::string													generateSessionId();
	void														cleanupExpiredSessions();
	std::map<std::string,std::string>							parseCookies(const std::string &cookieHeader);
	Session*													getOrCreateSession(const std::map<std::string,std::string> &cookies, HttpResponse &resp);
};

#endif