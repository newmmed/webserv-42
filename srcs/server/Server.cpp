/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 19:54:49 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/16 05:31:19 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/server/Server.hpp"
#include "../../includes/http/HttpParser.hpp"
#include "../../includes/http/HttpResponse.hpp"
#include "../../includes/http/HttpUtils.hpp"
#include "../../includes/http/HttpRouter.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>

std::string	Server::generateSessionId()
{
	unsigned char	raw[SESSION_ID_BYTES];
	int				fd = ::open("/dev/urandom", O_RDONLY);
	if (fd >= 0)
	{
		ssize_t	r = ::read(fd, raw, SESSION_ID_BYTES);
		::close(fd);
		if (r == static_cast<ssize_t>(SESSION_ID_BYTES))
		{
			std::ostringstream	oss;
			for (size_t i = 0; i < SESSION_ID_BYTES; ++i)
			{
				static const char	*hex = "0123456789abcdef";
				oss << hex[(raw[i] >> 4) & 0xF] << hex[raw[i] & 0xF];
			}
			return oss.str();
		}
	}
	return std::string("");
}

void	Server::cleanupExpiredSessions()
{
	time_t	now = time(NULL);
	for (std::map<std::string, Session>::iterator it = _sessions.begin(); it != _sessions.end(); )
	{
		if (it->second.expiresAt <= now)
			_sessions.erase(it++);
		else
			++it;
	}
}

std::map<std::string,std::string>	Server::parseCookies(const std::string &cookieHeader)
{
	std::map<std::string,std::string>	result;
	size_t								pos = 0;
	while (pos < cookieHeader.size())
	{
		size_t		semi = cookieHeader.find(';', pos);
		std::string	part;
		if (semi != std::string::npos)
			part = cookieHeader.substr(pos, semi-pos);
		else
			part = cookieHeader.substr(pos, std::string::npos);
		if (semi == std::string::npos)
			pos = cookieHeader.size();
		else
			pos = semi + 1;
		size_t		b = 0;
		while (b < part.size() && (part[b] == ' ' || part[b] == '\t'))
			++b;
		size_t		e = part.size();
		while (e > b && (part[e - 1] == ' ' || part[e - 1] == '\t'))
			--e;
		if (b >= e)
			continue ;
		std::string	token = part.substr(b, e - b);
		size_t		eq = token.find('=');
		if (eq == std::string::npos)
			continue ;
		std::string	name = token.substr(0, eq);
		std::string	value = token.substr(eq+1);
		if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"')
			value = value.substr(1, value.size() - 2);
		result[name] = value;
	}
	return result;
}

Session*	Server::getOrCreateSession(const std::map<std::string,std::string> &cookies, HttpResponse &resp)
{
	cleanupExpiredSessions();
	std::map<std::string,std::string>::const_iterator	it = cookies.find("sid");
	if (it != cookies.end())
	{
		std::map<std::string, Session>::iterator	sit = _sessions.find(it->second);
		if (sit != _sessions.end() && sit->second.expiresAt > time(NULL))
		{
			sit->second.expiresAt = time(NULL) + SESSION_TTL_SECONDS;
			return &sit->second;
		}
	}
	std::string	sid = generateSessionId();
	while (_sessions.find(sid) != _sessions.end())
		sid = generateSessionId();
	Session	s;
	s.id = sid;
	s.expiresAt = time(NULL) + SESSION_TTL_SECONDS;
	s.visitCount = 0;
	s.name = "";
	_sessions[sid] = s;
	std::ostringstream	attrs;
	attrs << "Path=/; HttpOnly; SameSite=Lax; Max-Age=" << SESSION_TTL_SECONDS;
	resp.addSetCookie("sid", sid, attrs.str());
	return &_sessions[sid];
}

Server::Server(Config &config, std::vector<std::pair<int, std::pair<std::string, int> > > allListens, int serverCount) : _conf(config), _allListens(allListens), _serverCount(serverCount)
{
	openListeningSocks();
}

void	Server::startServer()
{
	std::cout << "Server has been started!" << std::endl;
	while (86)
	{
		std::vector<struct pollfd>	polls;
		for (size_t i = 0; i < _listeningSocketsCount; i++)
		{
			struct pollfd	pfd;
			pfd.fd = _socks[i]._fd;
			pfd.events = POLLIN;
			pfd.revents = 0;
			polls.push_back(pfd);
		}
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			struct pollfd	pfd;
			pfd.fd = it->first;
			if (!it->second.resSent)
			{
				if (!it->second.responseBuffer.empty())
					pfd.events = POLLOUT;
				else
					pfd.events = POLLIN;
			}
			else
				pfd.events = POLLIN;
			pfd.revents = 0;
			polls.push_back(pfd);
		}
		int	pRet = poll(polls.data(), static_cast<nfds_t>(polls.size()), 1000);
		if (pRet < 0)
		{
			if (errno == EINTR)
				continue ;
			throw std::runtime_error("webserv: poll() failure in the main event loop");
		}
		timeoutChecker();
		if (!pRet)
			continue ;
		for (size_t i = 0; i < polls.size(); i++)
		{
			if (!polls[i].revents)
				continue ;
			if (i < _listeningSocketsCount && (polls[i].revents & POLLIN))
				acceptNewClient(_socks[i]);
			else if (i >= _listeningSocketsCount)
			{
				int	clientFd = polls[i].fd;
				if ((polls[i].revents & POLLERR) || (polls[i].revents & POLLHUP ) || (polls[i].revents & POLLNVAL))
				{
					deleteClient(clientFd);
					continue ;
				}
				if (polls[i].revents & POLLIN)
					handleClientInput(_clients.find(clientFd)->second);
				if (polls[i].revents & POLLOUT)
					handleClientOutput(_clients.find(clientFd)->second);
			}
		}
	}
}

void	Server::openListeningSocks()
{
	std::vector<ServerConfig>::iterator										it = _conf.configs.begin();
	std::vector<std::pair<int, std::pair<std::string, int> > >::iterator	it2 = _allListens.begin();
	for (int i = 1; i <= _serverCount; i++)
	{
		while (it2 != _allListens.end() && it2->first == i)
		{
			int	sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0)
				throw std::runtime_error("webserv: Failed to create socket");
			handleSock(sockfd, it2->second, T_SERVER);
			_socks.push_back(ListeningSocket(*it, it2->second.first, it2->second.second, sockfd));
			++it2;
		}
		if (i != _serverCount)
			++it;
	}
	_listeningSocketsCount = _socks.size();
}

void	Server::handleSock(int sockfd, std::pair<std::string, int> bindInfo, E_SOCKTYPE type)
{
	if (type == T_SERVER)
	{
		if (setRUASock(sockfd) < 0)
			throw std::runtime_error("webserv: Failed to set socket SO_REUSEADDR option");
		if (setNBSock(sockfd) < 0)
			throw std::runtime_error("webserv: Failed to set file descriptor non-blocking");
		bindAndListen(sockfd, bindInfo);
	}
}

int	Server::setRUASock(int sockfd)
{
	int	opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return (-1);
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		return (-1);
	return (0);
}

int	Server::setNBSock(int sockfd)
{
	int	flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0)
		return (-1);
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
		return (-1);
	return (0);
}

void	Server::bindAndListen(int sockfd, std::pair<std::string, int> bindInfo)
{
	struct sockaddr_in	addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(bindInfo.second);
	if (bindInfo.first == "0.0.0.0")
		addr.sin_addr.s_addr = INADDR_ANY;
	else
		inet_pton(AF_INET, bindInfo.first.c_str(), &addr.sin_addr);
	if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
		throw std::runtime_error("webserv: Failed to bind socket");
	if (listen(sockfd, SOMAXCONN) < 0)
		throw std::runtime_error("webserv: listen() failed");
}

void	Server::timeoutChecker()
{
	time_t				currentTime;
	const time_t		timeout = 60;
	std::vector<int>	toClose;

	currentTime = time(NULL);
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (currentTime - it->second.timeout > timeout)
			toClose.push_back(it->first);
	}
	for (size_t i = 0; i < toClose.size(); i++)
		deleteClient(toClose[i]);
}

void	Server::acceptNewClient(ListeningSocket &acceptingSocket)
{
	socklen_t			clientAddrLen;
	struct sockaddr_in	clientAddr;
	int					clientFd;
	char				address[INET_ADDRSTRLEN];
	int					clientPort;

	clientAddrLen = sizeof(clientAddr);
	clientFd = accept(acceptingSocket._fd, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientAddrLen);
	if (clientFd < 0)
	{
		std::cerr << "webserv: accept() failed to accept new client" << std::endl;
		return ;
	}
	if (setNBSock(clientFd) < 0)
	{
		std::cerr << "webserv: Failed to set client's file descriptor non-blocking" << std::endl;
		return ;
	}
	inet_ntop(AF_INET, reinterpret_cast<const void *>(&clientAddr.sin_addr.s_addr), address, INET_ADDRSTRLEN);
	clientPort = ntohs(clientAddr.sin_port);
	Client				newClient = Client(clientFd, address, clientPort, acceptingSocket);
	_clients.insert(std::make_pair(clientFd, newClient));
}

void	Server::deleteClient(int clientFd)
{
	_clients.erase(clientFd);
	close(clientFd);
}

static bool	readWholeFile(const std::string &path, std::string &out)
{
	FILE	*fp = std::fopen(path.c_str(), "rb");
	if (!fp)
		return false;
	char	buf[4096];
	size_t	n;
	out.clear();
	while ((n = std::fread(buf, 1, sizeof(buf), fp)) > 0)
		out.append(buf, n);
	std::fclose(fp);
	return true;
}

static bool	isDirectory(const std::string &path)
{
	struct stat	st;

	if (stat(path.c_str(), &st) != 0)
		return false;
	return S_ISDIR(st.st_mode);
}

static std::string	getExtension(const std::string &path)
{
	size_t	dot = path.find_last_of('.');

	if (dot == std::string::npos)
		return std::string();
	return path.substr(dot);
}

static void	addEnv(std::vector<std::string> &envv, const std::string &k, const std::string &v)
{
	std::string	kv = k + "=" + v;
	envv.push_back(kv);
}

static bool	runCgi(const std::string &interpreter, const std::string &scriptPath, const Client &client, const LocationConfig * /*loc*/, std::string &cgiOut)
{
	int	inpipe[2];
	int	outpipe[2];

	if (pipe(inpipe) < 0)
		return false;
	if (pipe(outpipe) < 0)
	{
		close(inpipe[0]);
		close(inpipe[1]);
		return false;
	}

	pid_t	pid = fork();
	if (pid < 0)
	{
		close(inpipe[0]);
		close(inpipe[1]);
		close(outpipe[0]);
		close(outpipe[1]);
		return false;
	}
	if (pid == 0)
	{
		dup2(inpipe[0], STDIN_FILENO);
		dup2(outpipe[1], STDOUT_FILENO);
		close(inpipe[0]);
		close(inpipe[1]);
		close(outpipe[0]);
		close(outpipe[1]);
		std::vector<std::string>	envv;
		addEnv(envv, "GATEWAY_INTERFACE", "CGI/1.1");
		addEnv(envv, "SERVER_PROTOCOL", client.version.empty() ? "HTTP/1.1" : client.version);
		addEnv(envv, "REQUEST_METHOD", client.method);
		addEnv(envv, "QUERY_STRING", client.query);
		addEnv(envv, "SCRIPT_FILENAME", scriptPath);
		addEnv(envv, "SCRIPT_NAME", client.path);
		addEnv(envv, "SERVER_NAME", client.acceptingSock._host);
		{
			char	buf[16];
			std::sprintf(buf, "%d", client.acceptingSock._port);
			addEnv(envv, "SERVER_PORT", buf);
		}
		addEnv(envv, "REMOTE_ADDR", client.addr.first);
		std::map<std::string,std::string>::const_iterator	itcl = client.headers.find("content-length");
		if (itcl != client.headers.end())
			addEnv(envv, "CONTENT_LENGTH", itcl->second);
		std::map<std::string,std::string>::const_iterator	itct = client.headers.find("content-type");
		if (itct != client.headers.end())
			addEnv(envv, "CONTENT_TYPE", itct->second);
		if (interpreter.find("php-cgi") != std::string::npos || getExtension(scriptPath) == ".php")
			addEnv(envv, "REDIRECT_STATUS", "200");
		for (std::map<std::string,std::string>::const_iterator it = client.headers.begin(); it != client.headers.end(); ++it)
		{
			std::string name = it->first;
			for (size_t i = 0; i < name.size(); ++i)
			{
				if (name[i] == '-')
					name[i] = '_';
				else
					name[i] = std::toupper(static_cast<unsigned char>(name[i]));
			}
			addEnv(envv, std::string("HTTP_") + name, it->second);
		}
		std::vector<char*>	envp;
		envp.reserve(envv.size() + 1);
		for (size_t i = 0; i < envv.size(); ++i)
			envp.push_back(const_cast<char*>(envv[i].c_str()));
		envp.push_back(NULL);
		char *const	argv[] = { const_cast<char*>(interpreter.c_str()), const_cast<char*>(scriptPath.c_str()), NULL };
		execve(interpreter.c_str(), argv, &envp[0]);
		_exit(127);
	}
	close(inpipe[0]);
	close(outpipe[1]);
	if (!client.body.empty())
	{
		size_t	off = 0;
		while (off < client.body.size())
		{
			ssize_t	w = write(inpipe[1], client.body.data() + off, client.body.size() - off);
			if (w < 0)
			{
				if (errno == EINTR)
					continue ;
				break ;
			}
			off += w;
		}
	}
	close(inpipe[1]);
	cgiOut.clear();
	char	buf[4096];
	ssize_t	n;
	while ((n = read(outpipe[0], buf, sizeof(buf))) > 0)
		cgiOut.append(buf, n);
	close(outpipe[0]);
	int		status = 0;
	waitpid(pid, &status, 0);
	return true;
}

static std::string	generateAutoIndexHTML(const std::string &dirPath, const std::string &urlPath)
{
	std::string	html = "<html><head><title>Index of ";
	html += urlPath;
	html += "</title></head><body><h1>Index of ";
	html += urlPath;
	html += "</h1><ul>";
	DIR			*d = opendir(dirPath.c_str());
	if (d)
	{
		struct dirent	*de;
		while ((de = readdir(d)))
		{
			std::string	name = de->d_name;
			if (name == "." || name == "..")
				continue ;
			html += "<li><a href=\"";
			html += http::joinPath(urlPath, name);
			html += "\">";
			html += name;
			html += "</a></li>";
		}
		closedir(d);
	}
	html += "</ul></body></html>";
	return html;
}

static void	applyErrorPage(ServerConfig &servConf, int code, HttpResponse &resp)
{
	std::string	errorRel = servConf.getErrorPage(code);
	if (!errorRel.empty())
	{
		std::string	path = http::joinPath(servConf.root, errorRel);
		std::string	body;
		if (readWholeFile(path, body))
		{
			resp.body = body;
			resp.headers["Content-Type"] = "text/html";
			return ;
		}
	}
	resp.body = "<html><body><h1>" + resp.reason + "</h1></body></html>";
	resp.headers["Content-Type"] = "text/html";
}

void	Server::handleClientInput(Client &client)
{
	char	clientRecvBuffer[1024];
	ssize_t	ret;

	ret = recv(client.fd, clientRecvBuffer, sizeof(clientRecvBuffer) - 1, 0);
	if (ret < 0)
		return ;
	if (!ret)
	{
		deleteClient(client.fd);
		return ;
	}
	client.timeout = time(NULL);
	client.requestBuffer.append(clientRecvBuffer, ret);
	client.bytesReceived += ret;
	if (!client.requestComplete)
	{
		if (HttpParser::parse(client))
		{
			HttpResponse								resp;
			std::map<std::string,std::string>			cookies;
			std::map<std::string,std::string>::iterator	hcookie = client.headers.find("cookie");
			if (hcookie != client.headers.end())
				cookies = parseCookies(hcookie->second);
			Session	*session = getOrCreateSession(cookies, resp);
			if (client.method == "GET" && client.path == "/session-api")
			{
				if (session)
					session->visitCount++;
				resp.statusCode = 200;
				resp.reason = httpReason(200);
				std::ostringstream	body;
				body << "{\n  \"sid\": \"" << (session? session->id:"") << "\",\n  \"visits\": " << (session? session->visitCount:0) << ",\n  \"name\": \"" << (session? session->name:"") << "\"\n}";
				resp.body = body.str();
				resp.headers["Content-Type"] = "application/json";
				client.responseBuffer = resp.serialize(client.keepAlive);
				return;
			}
			if (client.path == "/session-name")
			{
				if (!session)
				{
					resp.statusCode = 500;
					resp.reason = httpReason(500);
					resp.body = "Session is unavailable";
					resp.headers["Content-Type"] = "text/plain";
					client.responseBuffer = resp.serialize(client.keepAlive);
					return ;
				}
				if (client.method == "GET")
				{
					resp.statusCode = 200;
					resp.reason = httpReason(200);
					std::ostringstream	body;
					body << "{\n  \"name\": \"" << session->name << "\"\n}";
					resp.body = body.str();
					resp.headers["Content-Type"] = "application/json";
					client.responseBuffer = resp.serialize(client.keepAlive);
					return ;
				}
				if (client.method == "POST")
				{
					std::string	raw = client.body;
					while (!raw.empty() && (raw[0] == ' ' || raw[0] == '\t' || raw[0] == '\r' || raw[0] == '\n'))
						raw.erase(0, 1);
					while (!raw.empty() && (raw[raw.size() - 1] == ' ' || raw[raw.size() - 1] == '\t' || raw[raw.size() - 1] == '\r' || raw[raw.size() - 1] == '\n'))
						raw.erase(raw.size() - 1);
					if (!raw.empty())
					{
						for (size_t i = 0; i < raw.size(); ++i)
							if (raw[i] == '\r' || raw[i] == '\n')
								raw[i]=' ';
					}
					session->name = raw;
					resp.statusCode = 200;
					resp.reason = httpReason(200);
					resp.body = "OK";
					resp.headers["Content-Type"] = "text/plain";
					client.responseBuffer = resp.serialize(client.keepAlive);
					return ;
				}
				resp.statusCode = 405;
				resp.reason = httpReason(405);
				resp.headers["Allow"] = "GET, POST";
				resp.body = "";
				client.responseBuffer = resp.serialize(client.keepAlive);
				return ;
			}
			ServerConfig	&servConf = client.acceptingSock._sconf;
			RouteResult		rr = HttpRouter::route(servConf, client.path);
			LocationConfig	*loc = rr.location;
			std::string		effectiveRoot = loc ? loc->getRoot(servConf.root) : servConf.root;
			if (loc && loc->hasRedirection())
			{
				resp.statusCode = loc->redirectionCode;
				resp.reason = httpReason(resp.statusCode);
				resp.headers["Location"] = loc->redirectTo;
				client.responseBuffer = resp.serialize(client.keepAlive);
				return ;
			}
			if (loc && !loc->isMethodAllowed(client.method))
			{
				resp.statusCode = 405;
				resp.reason = httpReason(405);
				std::string	allow;
				for (size_t i = 0; i < loc->allowedMethods.size(); ++i)
				{
					if (i)
						allow += ", ";
					allow += loc->allowedMethods[i];
				}
				if (allow.empty())
					allow = "GET, POST, DELETE";
				resp.headers["Allow"] = allow;
				applyErrorPage(servConf, resp.statusCode, resp);
			}
			else
			{
				if (client.method == "GET")
				{
					std::string	relUrl = client.path;
					if (loc && !loc->path.empty())
					{
						if (relUrl.size() >= loc->path.size())
							relUrl = relUrl.substr(loc->path.size());
					}
					std::string	fullPath = http::joinPath(effectiveRoot, relUrl);
					std::string	ext = getExtension(fullPath);
					if (loc && !ext.empty())
					{
						std::map<std::string,std::string>::iterator	itcgi = const_cast<std::map<std::string,std::string>&>(loc->cgiExtensions).find(ext);
						if (itcgi != loc->cgiExtensions.end())
						{
							if (!http::isSafePath(fullPath))
							{
								resp.statusCode = 403;
								resp.reason = httpReason(403);
								applyErrorPage(servConf, resp.statusCode, resp);
							}
							else
							{
								std::string	out;
								if (!runCgi(itcgi->second, fullPath, client, loc, out))
								{
									resp.statusCode = 500;
									resp.reason = httpReason(500);
									applyErrorPage(servConf, resp.statusCode, resp);
								}
								else
								{
									size_t								hdrEnd = out.find("\r\n\r\n");
									bool								crlf = true;
									if (hdrEnd == std::string::npos)
									{
										hdrEnd = out.find("\n\n");
										crlf = false;
									}
									std::map<std::string, std::string>	cgih;
									size_t								pos = 0;
									if (hdrEnd != std::string::npos)
									{
										while (pos < hdrEnd)
										{
											size_t		eol = out.find(crlf ? "\r\n" : "\n", pos);
											if (eol == std::string::npos || eol > hdrEnd)
												break ;
											std::string	line = out.substr(pos, eol - pos);
											pos = eol + (crlf ? 2 : 1);
											size_t		c = line.find(':');
											if (c != std::string::npos)
											{
												std::string	n = http::toLower(http::trim(line.substr(0, c)));
												std::string	v = http::trim(line.substr(c + 1));
												cgih[n] = v;
											}
										}
										size_t	bodyStart = hdrEnd + (crlf ? 4 : 2);
										resp.body.assign(out.data() + bodyStart, out.size() - bodyStart);
									}
									else
										resp.body = out;
									std::map<std::string,std::string>::iterator	its = cgih.find("status");
									if (its != cgih.end())
									{
										int code = std::atoi(its->second.c_str());
										if (code < 100 || code > 599)
											code = 200;
										resp.statusCode = code;
										resp.reason = httpReason(code);
									}
									else
									{
										resp.statusCode = 200;
										resp.reason = httpReason(200);
									}
									std::map<std::string,std::string>::iterator	itcth = cgih.find("content-type");
									if (itcth != cgih.end())
										resp.headers["Content-Type"] = itcth->second;
									else
										resp.headers["Content-Type"] = "text/plain";
									for (std::map<std::string,std::string>::iterator ih = cgih.begin(); ih != cgih.end(); ++ih)
									{
										if (ih->first == "status" || ih->first == "content-type")
											continue ;
										std::string	name = ih->first;
										name[0] = std::toupper(name[0]);
										for (size_t i = 1; i < name.size(); ++i)
											if (name[i] == '-' && isalpha(name[i + 1]))
											{
												name[i + 1] = std::toupper(name[i + 1]);
											}
										resp.headers[name] = ih->second;
									}
								}
							}
							client.responseBuffer = resp.serialize(client.keepAlive);
							return ;
						}
					}
					if (isDirectory(fullPath))
					{
						bool						served = false;
						std::vector<std::string>	idxs = loc ? loc->indexes : std::vector<std::string>();
						for (size_t i = 0; i < idxs.size(); ++i)
						{
							std::string	idxPath = http::joinPath(fullPath, idxs[i]);
							std::string	tmp;
							if (readWholeFile(idxPath, tmp))
							{
								resp.body = tmp;
								resp.headers["Content-Type"] = http::guessContentType(idxPath);
								served = true;
								break ;
							}
						}
						if (!served)
						{
							if (loc && loc->autoIndex)
							{
								resp.body = generateAutoIndexHTML(fullPath, client.path);
								resp.headers["Content-Type"] = "text/html";
							}
							else
							{
								resp.statusCode = 404;
								resp.reason = httpReason(404);
								applyErrorPage(servConf, resp.statusCode, resp);
							}
						}
					}
					else
					{
						if (!http::isSafePath(fullPath))
						{
							resp.statusCode = 403;
							resp.reason = httpReason(403);
							applyErrorPage(servConf, resp.statusCode, resp);
						}
						else
						{
							std::string	data;
							if (!readWholeFile(fullPath, data))
							{
								resp.statusCode = 404;
								resp.reason = httpReason(404);
								applyErrorPage(servConf, resp.statusCode, resp);
							}
							else
							{
								resp.body = data;
								resp.headers["Content-Type"] = http::guessContentType(fullPath);
							}
						}
					}
				}
				else if (client.method == "POST")
				{
					size_t	allowed = loc ? loc->getBodySize(servConf.clientMaxBodySize) : servConf.clientMaxBodySize;
					if (client.body.size() > allowed)
					{
						resp.statusCode = 413;
						resp.reason = httpReason(413);
						applyErrorPage(servConf, resp.statusCode, resp);
					}
					else if (loc)
					{
						std::string	relUrl = client.path;
						if (loc && !loc->path.empty())
						{
							if (relUrl.size() >= loc->path.size())
								relUrl = relUrl.substr(loc->path.size());
						}
						std::string									fullPath = http::joinPath(effectiveRoot, relUrl);
						std::string									ext = getExtension(fullPath);
						std::map<std::string,std::string>::iterator	itcgi = const_cast<std::map<std::string,std::string>&>(loc->cgiExtensions).find(ext);
						if (itcgi != loc->cgiExtensions.end())
						{
							if (!http::isSafePath(fullPath))
							{
								resp.statusCode = 403;
								resp.reason = httpReason(403);
								applyErrorPage(servConf, resp.statusCode, resp);
							}
							else
							{
								std::string	out;
								if (!runCgi(itcgi->second, fullPath, client, loc, out))
								{
									resp.statusCode = 500;
									resp.reason = httpReason(500);
									applyErrorPage(servConf, resp.statusCode, resp);
								}
								else
								{
									size_t	hdrEnd = out.find("\r\n\r\n"); 
									bool	crlf = true;
									if (hdrEnd == std::string::npos)
									{
										hdrEnd = out.find("\n\n");
										crlf = false;
									}
									std::map<std::string,std::string>	cgih;
									size_t								pos = 0;
									if (hdrEnd != std::string::npos)
									{
										while (pos < hdrEnd)
										{
											size_t	eol = out.find(crlf ? "\r\n" : "\n", pos);
											if (eol == std::string::npos || eol > hdrEnd)
												break ;
											std::string	line = out.substr(pos, eol - pos);
											pos = eol + (crlf ? 2 : 1);
											size_t	c = line.find(':');
											if (c != std::string::npos)
											{
												std::string	n = http::toLower(http::trim(line.substr(0, c)));
												std::string	v = http::trim(line.substr(c + 1));
												cgih[n] = v;
											}
										}
										size_t	bodyStart = hdrEnd + (crlf ? 4 : 2);
										resp.body.assign(out.data() + bodyStart, out.size() - bodyStart);
									}
									else
										resp.body = out;
									std::map<std::string,std::string>::iterator	its = cgih.find("status");
									if (its != cgih.end())
									{
										int	code = std::atoi(its->second.c_str());
										if (code < 100 || code > 599)
											code = 200;
										resp.statusCode = code;
										resp.reason = httpReason(code);
									}
									else
									{
										resp.statusCode = 200;
										resp.reason = httpReason(200);
									}
									std::map<std::string,std::string>::iterator	itcth = cgih.find("content-type");
									if (itcth != cgih.end())
										resp.headers["Content-Type"] = itcth->second;
									else
										resp.headers["Content-Type"] = "text/plain";
									for (std::map<std::string,std::string>::iterator ih = cgih.begin(); ih != cgih.end(); ++ih)
									{
										if (ih->first == "status" || ih->first == "content-type")
											continue ;
										std::string	name = ih->first;
										resp.headers[name] = ih->second;
									}
								}
							}
							client.responseBuffer = resp.serialize(client.keepAlive);
							return ;
						}
					}
					if (loc && loc->uploadPerm && !loc->uploadTo.empty())
					{
						std::string	fileName;
						std::map<std::string,std::string>::iterator	hfn = client.headers.find("x-filename");
						if (hfn != client.headers.end() && !hfn->second.empty())
						{
							std::string	raw = http::urlDecode(hfn->second);
							size_t		slash = raw.find_last_of("/\\");
							if (slash != std::string::npos)
								raw = raw.substr(slash + 1);
							fileName.reserve(raw.size());
							for (size_t i = 0; i < raw.size(); ++i)
							{
								unsigned char	c = static_cast<unsigned char>(raw[i]);
								if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-')
									fileName.push_back(raw[i]);
								else
									fileName.push_back('_');
							}
							if (fileName == "." || fileName == ".." || fileName.empty())
								fileName.clear();
						}
						if (fileName.empty())
						{
							fileName = "upload_" + http::formatDateGmt();
							for (size_t i = 0; i < fileName.size(); ++i)
								if (fileName[i] == ' ' || fileName[i] == ':' || fileName[i] == ',')
									fileName[i]='_';
						}
						std::string	fullOut = http::joinPath(loc->uploadTo, fileName);
						if (!http::isSafePath(fullOut))
						{
							resp.statusCode = 403;
							resp.reason = httpReason(403);
							applyErrorPage(servConf, resp.statusCode, resp);
							client.responseBuffer = resp.serialize(client.keepAlive);
							return ;
						}
						for (int n = 1; n < 100 && std::fopen(fullOut.c_str(), "rb"); ++n)
						{
							FILE	*tmp = std::fopen(fullOut.c_str(), "rb");
							if (tmp)
								std::fclose(tmp);
							std::ostringstream	oss;
							oss << fileName << "-" << n;
							fullOut = http::joinPath(loc->uploadTo, oss.str());
						}
						FILE	*fp = std::fopen(fullOut.c_str(), "wb");
						if (!fp)
						{
							resp.statusCode = 500;
							resp.reason = httpReason(500);
							applyErrorPage(servConf, resp.statusCode, resp);
						}
						else
						{
							if (!client.body.empty())
							std::fwrite(client.body.data(), 1, client.body.size(), fp);
							std::fclose(fp);
							resp.statusCode = 201;
							resp.reason = httpReason(201);
							resp.headers["Location"] = http::joinPath(loc->path, fileName);
							resp.body = "Uploaded";
							resp.headers["Content-Type"] = "text/plain";
						}
					}
					else
					{
						resp.statusCode = 200;
						resp.reason = httpReason(200);
						resp.body = client.body;
						resp.headers["Content-Type"] = "text/plain";
					}
				}
				else if (client.method == "DELETE")
				{
					std::string	relUrl = client.path;
					if (loc && !loc->path.empty())
						if (relUrl.size() >= loc->path.size())
							relUrl = relUrl.substr(loc->path.size());
					std::string	fullPath = http::joinPath(effectiveRoot, relUrl);
					if (std::remove(fullPath.c_str()) == 0)
					{
						resp.statusCode = 200;
						resp.reason = httpReason(200);
						resp.body = "Deleted";
					}
					else
					{
						resp.statusCode = 404;
						resp.reason = httpReason(404);
						applyErrorPage(servConf, resp.statusCode, resp);
					}
				}
				else
				{
					resp.statusCode = 501;
					resp.reason = httpReason(501);
					applyErrorPage(servConf, resp.statusCode, resp);
				}
			}
			client.responseBuffer = resp.serialize(client.keepAlive);
		}
		else if (client.headersParsed && !client.requestComplete)
		{
			ServerConfig	&servConf = client.acceptingSock._sconf;
			RouteResult		rr = HttpRouter::route(servConf, client.path);
			LocationConfig	*loc = rr.location;
			size_t			allowed = loc ? loc->getBodySize(servConf.clientMaxBodySize) : servConf.clientMaxBodySize;
			if (client.expectedContentLength > allowed)
			{
				HttpResponse	resp;
				resp.statusCode = 413;
				resp.reason = httpReason(413);
				applyErrorPage(servConf, resp.statusCode, resp);
				client.responseBuffer = resp.serialize(false);
				client.requestComplete = true;
				client.keepAlive = false;
			}
		}
	}
}

void	Server::handleClientOutput(Client &client)
{
	if (client.responseBuffer.empty())
		return ;
	ssize_t	sent = send(client.fd, client.responseBuffer.data(), client.responseBuffer.size(), 0);
	if (sent < 0)
		return ;
	client.bytesSent += sent;
	if (static_cast<size_t>(sent) == client.responseBuffer.size())
	{
		client.resSent = true;
		if (!client.keepAlive)
			deleteClient(client.fd);
		else
		{
			client.requestBuffer.clear();
			client.responseBuffer.clear();
			client.body.clear();
			client.headers.clear();
			client.reqParsed = false;
			client.headersParsed = false;
			client.requestComplete = false;
			client.resSent = false;
			client.method.clear();
			client.requestTarget.clear();
			client.path.clear();
			client.query.clear();
			client.version.clear();
			client.expectedContentLength = 0;
			client.bytesReceived = 0;
			client.bytesSent = 0;
			client.timeout = time(NULL);
		}
	}
	else
		client.responseBuffer.erase(0, sent);
}
