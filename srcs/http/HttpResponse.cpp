#include "../../includes/http/HttpResponse.hpp"
#include "../../includes/http/HttpUtils.hpp"
#include <sstream>

// serialize: Produces a complete HTTP/1.1 response string. It guarantees
// essential headers if the caller didn't provide them:
//  - Date: RFC-1123 GMT timestamp
//  - Server: product identifier
//  - Content-Length: exact byte size of body
//  - Connection: keep-alive or close, depending on the argument
// Then appends any user headers as "Name: Value" lines, a blank line, and the
// raw body. The reason phrase defaults to httpReason(statusCode) if empty.
std::string	HttpResponse::serialize(bool keepAlive) const
{
	std::ostringstream	oss;

	oss << "HTTP/1.1 " << statusCode << ' ' << (reason.empty()? httpReason(statusCode): reason) << "\r\n";
	// Ensure essential headers
	if (headers.find("Date") == headers.end())
		oss << "Date: " << http::formatDateGmt() << "\r\n";
	if (headers.find("Server") == headers.end())
		oss << "Server: webserv/0.1" << "\r\n";
	if (headers.find("Content-Length") == headers.end())
		oss << "Content-Length: " << body.size() << "\r\n";
	if (headers.find("Connection") == headers.end())
		oss << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		oss << it->first << ": " << it->second << "\r\n";
	oss << "\r\n";
	oss << body;
	return oss.str();
}
