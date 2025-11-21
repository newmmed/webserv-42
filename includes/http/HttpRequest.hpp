#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>

struct HttpRequest
{
	std::string							method;
	std::string							target;
	std::string							path;  
	std::string							query; 
	std::string							version;
	std::map<std::string, std::string>	headers;
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

#endif