#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>

struct HttpRequest
{
	std::string							method;     // GET, POST, DELETE
	std::string							target;     // Raw request-target (path + query)
	std::string							path;       // Decoded path
	std::string							query;      // Query part without '?'
	std::string							version;    // HTTP/1.0 or HTTP/1.1
	std::map<std::string, std::string>	headers; // lower-cased keys
	std::string							body;

	void clear()
	{
		method.clear();
		target.clear();
		path.clear();
		query.clear();
		version.clear();
		headers.clear();
		body.clear();
	}
};

#endif // HTTP_REQUEST_HPP
