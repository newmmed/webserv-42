#ifndef HTTP_ROUTER_HPP
# define HTTP_ROUTER_HPP

# include <string>
# include "../config-parser/Config.hpp"

struct RouteResult {
	ServerConfig* server;
	LocationConfig* location;
};

class HttpRouter
{
public:
	static RouteResult	route(ServerConfig &server, const std::string &path);
};

#endif // HTTP_ROUTER_HPP
