#ifndef HTTP_UTILS_HPP
# define HTTP_UTILS_HPP

# include <string>
# include <map>
# include <vector>

namespace http
{
	std::string	toLower(const std::string& s);
	std::string	trim(const std::string& s);
	bool		iequals(const std::string& a, const std::string& b);
	std::string	urlDecode(const std::string& in);
	std::string	joinPath(const std::string& a, const std::string& b);
	bool		isSafePath(const std::string& path);
	std::string	guessContentType(const std::string& path);
	std::string	formatDateGmt();
}

#endif