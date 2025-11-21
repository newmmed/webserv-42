/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 08:28:45 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/16 01:03:39 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/config-parser/Config.hpp"

LocationConfig::LocationConfig() : path(""), root(""), uploadTo(""), redirectTo(""), uploadPerm(false), autoIndex(false), redirectionCode(0), clientMaxBodySize(0)
{
	allowedMethods.push_back("GET");
	allowedMethods.push_back("POST");
	allowedMethods.push_back("DELETE");
	indexes.push_back("index.html");
	indexes.push_back("index.htm");
}

ServerConfig::ServerConfig() : root(getenv("PWD")), clientMaxBodySize(1048576)
{
	errorPages[400] = "/error_pages/400.html";
	errorPages[403] = "/error_pages/403.html";
	errorPages[404] = "/error_pages/404.html";
	errorPages[405] = "/error_pages/405.html";
	errorPages[500] = "/error_pages/50x.html";
	errorPages[502] = "/error_pages/50x.html";
	errorPages[503] = "/error_pages/50x.html";
	errorPages[504] = "/error_pages/50x.html";
}

bool	LocationConfig::hasRedirection() const
{
	return (redirectionCode != 0);
}

bool	LocationConfig::isMethodAllowed(const std::string &method) const
{
	for (size_t i = 0; i < allowedMethods.size(); i++)
	{
		if (method == allowedMethods[i])
			return (true);
	}
	return (false);
}

size_t	LocationConfig::getBodySize(size_t serverBodySize) const
{
	if (clientMaxBodySize != 0)
		return (clientMaxBodySize);
	return (serverBodySize);
}

std::string	LocationConfig::getRoot(const std::string &serverRoot) const
{
	if (!root.empty())
		return (root);
	return (serverRoot);
}

LocationConfig* ServerConfig::findLocation(const std::string &path)
{
	LocationConfig*	best = NULL;
	for (size_t i = 0; i < locations.size(); ++i)
	{
		const std::string	&locPath = locations[i].path;
		if (locPath.empty())
			continue;
		if (path.size() >= locPath.size() && path.compare(0, locPath.size(), locPath) == 0)
		{
			bool	boundaryOK = (locPath == "/") || path.size() == locPath.size() || path[locPath.size()] == '/';
			if (boundaryOK)
			{
				if (!best || locPath.size() > best->path.size())
					best = &locations[i];
			}
		}
	}
	return best;
}

std::string ServerConfig::getErrorPage(int statusCode) const
{
	std::map<int, std::string>::const_iterator	it = errorPages.find(statusCode);
	if (it != errorPages.end())
		return it->second;
	return std::string();
}
