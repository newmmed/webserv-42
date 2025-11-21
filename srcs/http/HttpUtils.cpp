#include "../../includes/http/HttpUtils.hpp"
#include <cctype>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace http
{
	std::string toLower(const std::string& s)
	{
		std::string	r;

		r.reserve(s.size());
		for (size_t i=0;i<s.size();++i)
			r.push_back(std::tolower(static_cast<unsigned char>(s[i])));
		return r;
	}

	static bool isSpace(char c)
	{
		return c==' ' || c=='\t' || c=='\r' || c=='\n' || c=='\f';
	}

	std::string trim(const std::string& s)
	{
		size_t	b = 0;

		while (b < s.size() && isSpace(s[b]))
			++b;
		if (b == s.size())
			return "";
		size_t	e = s.size() - 1;
		while (e > b && isSpace(s[e]))
			--e;
		return s.substr(b, e - b + 1);
	}

	bool iequals(const std::string& a,const std::string& b)
	{
		if (a.size() != b.size())
			return false;
		for (size_t i = 0; i < a.size(); ++i)
			if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
				return false;
		return true;
	}

	std::string urlDecode(const std::string& in)
	{
		std::string	out;

		out.reserve(in.size());
		for (size_t i = 0; i < in.size(); ++i)
		{
			if (in[i] == '%' && i + 2 < in.size() && std::isxdigit(in[i + 1]) && std::isxdigit(in[i + 2]))
			{
				int	hi = std::isdigit(in[i + 1]) ? in[i + 1] - '0' : std::tolower(in[i + 1]) - 'a' + 10;
				int	lo = std::isdigit(in[i + 2]) ? in[i + 2] - '0' : std::tolower(in[i + 2]) - 'a' + 10;
				out.push_back((char)((hi << 4) | lo));
				i += 2;
			}
			else if (in[i] == '+')
				out.push_back(' ');
			else
				out.push_back(in[i]);
		}
		return out;
	}

	std::string joinPath(const std::string& a,const std::string& b)
	{
		if (a.empty())
			return b;
		if (a[a.size() - 1] == '/')
			return a + (b.size() && b[0] == '/' ? b.substr(1) : b);
		return a + '/' + (b.size() && b[0]=='/' ? b.substr(1) : b);
	}

	bool isSafePath(const std::string& path)
	{
		size_t	pos = 0;
		while (pos < path.size())
		{
			size_t	next = path.find('/', pos);
			if (next == std::string::npos)
				next=path.size();
			if (next - pos == 2 && path[pos] == '.' && path[pos + 1] == '.')
				return false;
			pos = next + 1;
		}
		return true;
	}

	std::string guessContentType(const std::string& path)
	{
		size_t		dot = path.find_last_of('.');
		if (dot == std::string::npos)
			return "text/plain";
		std::string	ext = path.substr(dot + 1);
		for (size_t i = 0; i < ext.size(); ++i)
			ext[i] = std::tolower(static_cast<unsigned char>(ext[i]));
		if (ext == "html" || ext == "htm")
			return "text/html";
		if (ext == "css")
			return "text/css";
		if (ext == "js")
			return "application/javascript";
		if (ext == "png")
			return "image/png";
		if (ext == "jpg" || ext == "jpeg")
			return "image/jpeg";
		if (ext == "gif")
			return "image/gif";
		if (ext == "txt")
			return "text/plain";
		return "application/octet-stream";
	}

	std::string formatDateGmt()
	{
		std::time_t	t = std::time(NULL);
		std::tm		tmv;
		char		buf[128];

		gmtime_r(&t, &tmv);
		std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tmv);
		return std::string(buf);
	}
}
