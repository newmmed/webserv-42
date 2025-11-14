#include "../../includes/http/HttpParser.hpp"
#include "../../includes/http/HttpUtils.hpp"
#include "../../includes/http/HttpStatus.hpp"
#include <cstdlib>
#include <sstream>

/*
 How this parser works (why and how):
 - It parses incrementally from Client.requestBuffer without consuming bytes,
     so Server::handleClientInput can append more data and re-run parsing.
 - Phases: request-line -> headers -> finalize body expectation -> body.
 - It only supports fixed-length bodies via Content-Length (per project scope).
 - It sets keep-alive according to HTTP version and Connection header.
 - On parse error (bad request line or invalid Content-Length), it prepares a
     minimal 400 response directly into client.responseBuffer and marks the
     request as complete with keepAlive=false. This allows the main loop to send
     the error immediately without crashing or blocking other clients.
*/

// Helper to get header value (case-insensitive stored as lower keys)
// getHeader: helper that fetches a header value from a lower-cased map.
// Returns empty string if header is absent.
static std::string	getHeader(const std::map<std::string,std::string>& h, const std::string& k)
{
	std::map<std::string,std::string>::const_iterator	it = h.find(http::toLower(k));
	if (it == h.end())
		return std::string();
	return it->second;
}

// parse: Drives the phased parsing process. Returns true once a full request
// (start-line, headers, and body if any) is available. Returns false when
// more data is needed or after preparing an error response.
bool	HttpParser::parse(Client &client)
{
	// Parse request line
	if (!client.reqParsed)
	{
		if (!parseRequestLine(client))
			return false; // error might already be set in buffer
		client.reqParsed = true;
	}
	// Parse headers
	if (!client.headersParsed)
	{
		if (!parseHeaders(client))
			return false;
	}
	// Decide on body length/keepAlive if not finalized yet
	if (!client.requestComplete && client.headersParsed)
	{
		if (!finalizeBodyExpectation(client))
			return false;
	}
	// Parse body if any
	if (!client.requestComplete)
	{
		if (!parseBody(client))
			return false;
	}
	return client.requestComplete;
}

// parseRequestLine: Extracts METHOD, TARGET, VERSION from the first line.
// Splits TARGET into path and query, decoding the path. Does not erase data
// from requestBuffer (keeps indexes simple for later phases). Returns false
// if the line is incomplete; on format error, writes a 400 response.
bool	HttpParser::parseRequestLine(Client &client)
{
	size_t				lineEnd = client.requestBuffer.find("\r\n");
	if (lineEnd == std::string::npos)
		return false; // need more data
	std::string			line = client.requestBuffer.substr(0, lineEnd);
	// Remove the line from buffer? We'll keep buffer intact; indices are simple.
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
	// split path and query
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

// parseHeaders: Scans lines until CRLF CRLF and fills client.headers with
// lower-cased names and trimmed values. Returns false if headers are
// incomplete; does not validate semantics here.
bool	HttpParser::parseHeaders(Client &client)
{
	size_t	headerEnd = client.requestBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false; // need more data
	// Start after request line CRLF
	size_t	cur = client.requestBuffer.find("\r\n");
	if (cur == std::string::npos)
		return false; // should not happen after earlier check
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
			continue ; // skip bad header name silently
		std::string	name = http::toLower(http::trim(line.substr(0,colon)));
		std::string	value = http::trim(line.substr(colon+1));
		client.headers[name] = value;
	}
	client.headersParsed = true;
	return true;
}

// finalizeBodyExpectation: Interprets Content-Length (if present) and sets
// client.expectedContentLength. Decides keep-alive policy:
//  - HTTP/1.1: keep-alive unless Connection: close
//  - HTTP/1.0: close unless Connection: keep-alive
// On invalid Content-Length, emits a 400 response and aborts.
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
	// keep-alive decision (HTTP/1.1 default keep-alive unless close; HTTP/1.0 default close unless keep-alive)
	std::string	conn = getHeader(client.headers, "connection");
	if (client.version == "HTTP/1.1")
		client.keepAlive = !(http::iequals(conn, "close"));
	else
		client.keepAlive = http::iequals(conn, "keep-alive");
	return true;
}

// parseBody: After CRLF CRLF, waits for expectedContentLength bytes and copies
// exactly that many into client.body (binary-safe). Marks requestComplete.
bool	HttpParser::parseBody(Client &client)
{
	size_t	headerEnd = client.requestBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;
	size_t	have = client.requestBuffer.size() - (headerEnd + 4);
	if (have < client.expectedContentLength)
		return false; // need more
	if (client.expectedContentLength > 0)
		client.body.assign(client.requestBuffer.data() + headerEnd + 4, client.expectedContentLength);
	else
		client.body.clear();
	client.requestComplete = true;
	return true;
}
