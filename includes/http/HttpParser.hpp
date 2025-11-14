#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include <string>
# include <map>
# include "HttpRequest.hpp"
# include "../config-parser/Config.hpp"
# include "../server/Client.hpp"

class HttpParser
{
public:
	// Returns true when a full request (headers + body if any) is parsed
	// On error, fills responseBuffer with an error response and returns false with client.requestComplete=true
	static bool	parse(Client &client);

private:
	static bool	parseRequestLine(Client &client);
	static bool	parseHeaders(Client &client);
	static bool	parseBody(Client &client);
	static bool	finalizeBodyExpectation(Client &client);
};

#endif // HTTP_PARSER_HPP
