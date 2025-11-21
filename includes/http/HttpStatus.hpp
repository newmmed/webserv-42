#ifndef HTTP_STATUS_HPP
# define HTTP_STATUS_HPP

# include <string>

struct HttpStatus
{
	int			code;
	const char*	reason;
};

inline const char*	httpReason(int code)
{
	switch (code)
	{
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 204:
			return "No Content";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 400:
			return "Bad Request";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 411:
			return "Length Required";
		case 413:
			return "Payload Too Large";
		case 414:
			return "URI Too Long";
		case 431:
			return "Request Header Fields Too Large";
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		default:
			return "Unknown";
	}
}

#endif