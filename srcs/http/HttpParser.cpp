#include "../../includes/http/HttpParser.hpp"
#include "../../includes/http/HttpUtils.hpp"
#include "../../includes/http/HttpStatus.hpp"
#include <cstdlib>
#include <sstream>

static std::string	getHeader(const std::map<std::string,std::string>& h, const std::string& k)
{
	std::map<std::string,std::string>::const_iterator	it = h.find(http::toLower(k));
	if (it == h.end())
		return std::string();
	return it->second;
}

bool	HttpParser::parse(Client &client)
{
	if (!client.reqParsed)
	{
		if (!parseRequestLine(client))
			return false;
		client.reqParsed = true;
	}
	if (!client.headersParsed)
	{
		if (!parseHeaders(client))
			return false;
	}
	if (!client.requestComplete && client.headersParsed)
	{
		if (!finalizeBodyExpectation(client))
			return false;
	}
	if (!client.requestComplete)
	{
		if (!parseBody(client))
			return false;
	}
	return client.requestComplete;
}

bool	HttpParser::parseRequestLine(Client &client)
{
	size_t				lineEnd = client.requestBuffer.find("\r\n");
	if (lineEnd == std::string::npos)
		return false;
	std::string			line = client.requestBuffer.substr(0, lineEnd);
	std::istringstream	iss(line);
	std::string			method, target, version;
	if (!(iss >> method >> target >> version))
	{
		client.responseBuffer = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
		client.requestComplete = true;
		client.resSent = false;
		client.keepAlive=false;
		return false;
	}
	client.method = method;
	client.requestTarget = target;
	client.version = version;
	size_t				qpos = target.find('?');
	if (qpos==std::string::npos)
	{
		client.path = http::urlDecode(target);
		client.query = "";
	}
	else
	{
		client.path = http::urlDecode(target.substr(0, qpos));
		client.query = target.substr(qpos + 1);
	}
	return true;
}

bool	HttpParser::parseHeaders(Client &client)
{
	size_t	headerEnd = client.requestBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;
	size_t	cur = client.requestBuffer.find("\r\n");
	if (cur == std::string::npos)
		return false;
	cur += 2;
	while (cur < headerEnd)
	{
		size_t		eol = client.requestBuffer.find("\r\n", cur);
		if (eol == std::string::npos || eol > headerEnd)
			break ;
		std::string	line = client.requestBuffer.substr(cur, eol - cur);
		cur = eol + 2;
		size_t		colon = line.find(':');
		if (colon == std::string::npos)
			continue ;
		std::string	name = http::toLower(http::trim(line.substr(0,colon)));
		std::string	value = http::trim(line.substr(colon+1));
		client.headers[name] = value;
	}
	client.headersParsed = true;
	return true;
}

bool	HttpParser::finalizeBodyExpectation(Client &client)
{
	std::string	cl = getHeader(client.headers, "content-length");
	client.expectedContentLength = 0;
	if (!cl.empty())
	{
		char	*end = 0;
		long	v = std::strtol(cl.c_str(), &end, 10);
		if (*end != '\0' || v < 0)
		{
			client.responseBuffer = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
			client.requestComplete = true;
			client.keepAlive = false;
			return false;
		}
		client.expectedContentLength = static_cast<size_t>(v);
	}
	std::string	conn = getHeader(client.headers, "connection");
	if (client.version == "HTTP/1.1")
		client.keepAlive = !(http::iequals(conn, "close"));
	else
		client.keepAlive = http::iequals(conn, "keep-alive");
	return true;
}

bool	HttpParser::parseBody(Client &client)
{
	size_t	headerEnd = client.requestBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;
	size_t	have = client.requestBuffer.size() - (headerEnd + 4);
	if (have < client.expectedContentLength)
		return false;
	if (client.expectedContentLength > 0)
		client.body.assign(client.requestBuffer.data() + headerEnd + 4, client.expectedContentLength);
	else
		client.body.clear();
	client.requestComplete = true;
	return true;
}
