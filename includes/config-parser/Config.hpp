/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/09 23:03:55 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/16 01:08:51 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <map>
# include <vector>
# include <cstdlib>

typedef struct	LocationConfig
{
	std::string							path;
	std::string							root;
	std::string							uploadTo;
	std::string							redirectTo;
	std::vector<std::string>			allowedMethods;
	std::vector<std::string>			indexes;
	bool								uploadPerm;
	bool								autoIndex;
	int									redirectionCode;
	size_t								clientMaxBodySize;
	std::map<std::string, std::string>	cgiExtensions;
	LocationConfig();
	bool								hasRedirection() const;
	bool								isMethodAllowed(const std::string &method) const;
	size_t								getBodySize(size_t serverBodySize) const;
	std::string							getRoot(const std::string &serverRoot) const;
}				LocationConfig;

typedef struct	ServerConfig
{
	std::vector<std::pair<std::string, int> >	listens;
	std::vector<LocationConfig>					locations;
	std::map<int, std::string>					errorPages;
	std::string									root;
	size_t										clientMaxBodySize;
	ServerConfig();
	LocationConfig								*findLocation(const std::string &path);
	std::string									getErrorPage(int statusCode) const;
}				ServerConfig;

typedef struct	Config
{
	std::vector<ServerConfig>	configs;
}				Config;

#endif