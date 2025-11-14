#include "../../includes/http/HttpRouter.hpp"

// route: Delegates to ServerConfig::findLocation using the requested path and
// returns both the server and the matched location (if any). This isolates the
// server's request handling code from config traversal details.
RouteResult	HttpRouter::route(ServerConfig &server, const std::string &path)
{
	RouteResult	r;

	r.server = &server;
	r.location = server.findLocation(path);
	return r;
}
