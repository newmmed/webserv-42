#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <string>
# include <map>
# include <vector>
# include "HttpStatus.hpp"

struct HttpResponse
{
	int									statusCode;
	std::string							reason;
	std::map<std::string, std::string>	headers;
	std::string							body;
	std::vector<std::string>			setCookieHeaders;
	HttpResponse(): statusCode(200), reason(httpReason(200)) {}
	void								addSetCookie(const std::string &name, const std::string &value, const std::string &attrs);
	std::string							serialize(bool keepAlive) const;
};

#endif