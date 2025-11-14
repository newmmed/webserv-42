#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <string>
# include <map>
# include "HttpStatus.hpp"

struct HttpResponse
{
	int									statusCode;
	std::string							reason;
	std::map<std::string, std::string>	headers;
	std::string							body;
	HttpResponse(): statusCode(200), reason(httpReason(200)) {}
	std::string							serialize(bool keepAlive) const;
};

#endif // HTTP_RESPONSE_HPP
