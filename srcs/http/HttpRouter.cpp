#include "../../includes/http/HttpRouter.hpp"

RouteResult	HttpRouter::route(ServerConfig &server, const std::string &path)
{
	RouteResult	r;

	r.server = &server;
	r.location = server.findLocation(path);
	return r;
}
