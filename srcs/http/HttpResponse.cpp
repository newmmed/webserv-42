#include "../../includes/http/HttpResponse.hpp"
#include "../../includes/http/HttpUtils.hpp"
#include <sstream>

void	HttpResponse::addSetCookie(const std::string &name, const std::string &value, const std::string &attrs)
{
	std::ostringstream	oss;
	oss << name << "=" << value;
	if (!attrs.empty())
		oss << "; " << attrs;
	setCookieHeaders.push_back(oss.str());
}

std::string	HttpResponse::serialize(bool keepAlive) const
{
	std::ostringstream	oss;

	oss << "HTTP/1.1 " << statusCode << ' ' << (reason.empty()? httpReason(statusCode): reason) << "\r\n";
	if (headers.find("Date") == headers.end())
		oss << "Date: " << http::formatDateGmt() << "\r\n";
	if (headers.find("Server") == headers.end())
		oss << "Server: webserv/0.1" << "\r\n";
	if (headers.find("Content-Length") == headers.end())
		oss << "Content-Length: " << body.size() << "\r\n";
	if (headers.find("Connection") == headers.end())
		oss << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n";
	for (size_t i = 0; i < setCookieHeaders.size(); ++i)
		oss << "Set-Cookie: " << setCookieHeaders[i] << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		oss << it->first << ": " << it->second << "\r\n";
	oss << "\r\n";
	oss << body;
	return oss.str();
}
