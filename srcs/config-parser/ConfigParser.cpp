/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/04 19:47:00 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/13 09:58:59 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/config-parser/ConfigParser.hpp"

ConfigParser::ConfigParser(char *configFile) : _serverCount(0), _pState(GLOBAL_STATE), _configFileName(configFile)
{
	_tokenizer = Tokenizer(configFile);
	_configContent = _tokenizer.getConfig();
	Token	holder;
	while ((holder = _tokenizer.getNextToken()).getType() != T_EOF)
	{
		_tokens.push_back(holder);
		// std::cout << "Token value is: " << holder.getContent() << " of type: " << holder.getType() << " at line: " << holder.getLine() << ", posInLine: " << holder.getPosInLine() << std::endl;
	}
}

Config		ConfigParser::parseConfig()
{
	std::vector<Token>::const_iterator	it = _tokens.begin();
	while (it != _tokens.end())
	{
		switch (_pState)
		{
			case GLOBAL_STATE:
				handleGlobalState(it);
				break;
			case SERVER_STATE:
				handleServerState(it);
				break;
			case LOCATION_STATE:
				handleLocationState(it);
				break;
		}
	}
	validateConfig();
	return (_config);
}

void ConfigParser::handleGlobalState(std::vector<Token>::const_iterator &it)
{
	Token	token = *it;
	if (token._type == T_WORD && token._content == "server")
	{
		++it;
		expectToken(it, T_LEFT_BRACE, "Expected '{' after 'server'");
		_currentServer = ServerConfig();
		_pState = SERVER_STATE;
		_serverCount++;
	}
	else
		throw ParseException("Expected 'server' keyword", _configFileName, token._posInLine, token._line);
}

void ConfigParser::handleServerState(std::vector<Token>::const_iterator &it)
{
	Token		token = *it;
	if (token._type == T_RIGHT_BRACE)
	{
		_config.configs.push_back(_currentServer);
		_pState = GLOBAL_STATE;
		++it;
		return ;
	}
	if (token._type != T_WORD)
		throw ParseException("Expected directive or '}'", _configFileName, token._posInLine, token._line);
	std::string	directive = token._content;
	++it;
	switch (getServerDirective(directive))
	{
		case D_SERVER_LISTEN:
			parseListenDirective(it);
			break;
		case D_SERVER_ROOT:
			parseServerRootDirective(it);
			break;
		case D_SERVER_ERROR_PAGE:
			parseErrorPageDirective(it);
			break;
		case D_SERVER_CLIENT_MAX_BODY_SIZE:
			parseServerBodySizeDirective(it);
			break;
		case D_SERVER_LOCATION:
			if (it == _tokens.end() || it->_type != T_WORD)
				throw ParseException("Expected location path after 'location'", _configFileName, token._posInLine, token._line);
			_currentLocation = LocationConfig();
			_currentLocation.path = it->_content;
			++it;
			expectToken(it, T_LEFT_BRACE, "Expected '{' after location path");
			_pState = LOCATION_STATE;
			break;
		case D_SERVER_UNKNOWN:
			throw ParseException("Unknown server directive: " + directive, _configFileName, token._posInLine, token._line);
		default:
			throw ParseException("Unexpected directive in server context: " + directive, _configFileName, token._posInLine, token._line);
	}
}

void ConfigParser::handleLocationState(std::vector<Token>::const_iterator &it)
{
	Token		token = *it;
	if (token._type == T_RIGHT_BRACE)
	{
		_currentServer.locations.push_back(_currentLocation);
		_pState = SERVER_STATE;
		++it;
		return ;
	}
	if (token._type != T_WORD)
		throw ParseException("Expected directive or '}'", _configFileName, token._posInLine, token._line);
	std::string	directive = token._content;
	++it;
	switch (getLocationDirective(directive))
	{
		case D_LOC_ROOT:
			parseLocationRootDirective(it);
			break;
		case D_LOC_INDEX:
			parseIndexDirective(it);
			break;
		case D_LOC_ALLOW_METHODS:
			parseAllowMethodsDirective(it);
			break;
		case D_LOC_AUTOINDEX:
			parseAutoIndexDirective(it);
			break;
		case D_LOC_UPLOAD_ENABLE:
			parseUploadEnableDirective(it);
			break;
		case D_LOC_UPLOAD_STORE:
			parseUploadStoreDirective(it);
			break;
		case D_LOC_RETURN:
			parseReturnDirective(it);
			break;
		case D_LOC_CGI_EXTENSION:
			parseCgiExtensionDirective(it);
			break;
		case D_LOC_CLIENT_MAX_BODY_SIZE:
			parseLocationBodySizeDirective(it);
			break;
		case D_LOC_UNKNOWN:
			throw ParseException("Unknown location directive: " + directive, _configFileName, token._posInLine, token._line);
		default:
			throw ParseException("Unexpected directive in location context: " + directive, _configFileName, token._posInLine, token._line);
	}
}

void	ConfigParser::parseListenDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || (it->_type != T_WORD && it->_type != T_NUMBER))
		throw ParseException("Expected host and/or port after server's listen directive", _configFileName, it->_posInLine, it->_line);
	std::string	tokenValue = it->_content;
	if (it->_type == T_WORD)
	{
		size_t		valueColonPos = tokenValue.find(':');
		if (valueColonPos == std::string::npos)
			throw ParseException("Expected colon seperating host and port in listen directive's value", _configFileName, (it - 1)->_posInLine, (it - 1)->_line);
		std::string	host = tokenValue.substr(0, valueColonPos);
		std::string	port = tokenValue.substr(valueColonPos + 1);
		int			portInt = atoi(port.c_str());
		if (portInt < 0 || portInt > 65535)
			throw ParseException("Port: " + port + " is out of range, make sure it is between 0 and 65535", _configFileName, (it - 1)->_posInLine, (it - 1)->_line);
		for (size_t i = 0; i < _allListens.size(); i++)
		{
			if (portInt == _allListens[i].second.second && (_allListens[i].second.first == "0.0.0.0" || _allListens[i].second.first == host))
				throw ParseException("Port: " + tokenValue + " is duplicated, virtual hosts service isn't implemented yet", _configFileName, (it - 1)->_posInLine, (it - 1)->_line);
		}
		if (invalidHostIp(host))
			throw ParseException("Specified host IP: " + host + " is invalid", _configFileName, (it - 1)->_posInLine, (it - 1)->_line);
		_currentServer.listens.push_back(std::make_pair(host, portInt));
		_allListens.push_back(std::make_pair(_serverCount ,std::make_pair(host, portInt)));
	}
	else if (it->_type == T_NUMBER)
	{
		int	portInt = atoi(tokenValue.c_str());
		if (portInt < 0 || portInt > 65535)
			throw ParseException("Port: " + tokenValue + " is out of range, make sure it is between 0 and 65535", _configFileName, (it - 1)->_posInLine, (it - 1)->_line);
		for (size_t i = 0; i < _allListens.size(); i++)
		{
			if (portInt == _allListens[i].second.second)
				throw ParseException("Port: " + tokenValue + " is duplicated, virtual hosts service isn't implemented yet", _configFileName, (it - 1)->_posInLine, (it - 1)->_line);
		}
		_currentServer.listens.push_back(std::make_pair("0.0.0.0", portInt));
		_allListens.push_back(std::make_pair(_serverCount, std::make_pair("0.0.0.0", portInt)));
	}
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude server's listen directive");
}

void	ConfigParser::parseServerRootDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected path after server root directive", _configFileName, it->_posInLine, it->_line);
	_currentServer.root = it->_content;
	if (_currentServer.root[0] != '/')
		throw ParseException("Server's root directive value must start with a slash '/'", _configFileName, it->_posInLine, it->_line);
	if (!pathExists(_currentServer.root))
		throw ParseException("Server's root directive value must exist", _configFileName, it->_posInLine, it->_line);
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude server's root directive");
}

void	ConfigParser::parseErrorPageDirective(std::vector<Token>::const_iterator &it)
{
	std::vector<int>	errorStatusCodes;

	while (it != _tokens.end() && it->_type == T_NUMBER)
	{
		int	currentErrorStatus = atoi(it->_content.c_str());
		if (currentErrorStatus < 100 || currentErrorStatus > 599)
			throw ParseException(it->_content + " status code is invalid", _configFileName, it->_posInLine, it->_line);
		errorStatusCodes.push_back(currentErrorStatus);
		++it;
	}
	if (errorStatusCodes.empty())
		throw ParseException("Expected status code(s) after error_page server's directive", _configFileName, it->_posInLine, it->_line);
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected path to error page after error_page status codes", _configFileName, it->_posInLine, it->_line);
	std::string	pathToErrorPage = it->_content;
	++it;
	if (pathToErrorPage[0] != '/')
		throw ParseException("Path to an error page must start with a slash '/'", _configFileName, it->_posInLine, it->_line);
	for (size_t i = 0; i < errorStatusCodes.size(); i++)
		_currentServer.errorPages[errorStatusCodes[i]] = pathToErrorPage;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude server's error_page directive");
}

void	ConfigParser::parseServerBodySizeDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || (it->_type != T_WORD && it->_type != T_NUMBER))
		throw ParseException("Expected size after server's client_max_body_size directive", _configFileName, it->_posInLine, it->_line);
	std::string bodySize = it->_content;
	if (it->_type == T_NUMBER)
		_currentServer.clientMaxBodySize = atoi(bodySize.c_str());
	else if (it->_type == T_WORD)
		_currentServer.clientMaxBodySize = convertSize(it, bodySize);
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude server's client_max_body_size directive");
}

void	ConfigParser::parseLocationRootDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected path after location's root directive", _configFileName, it->_posInLine, it->_line);
	_currentLocation.root = it->_content;
	if (_currentLocation.root[0] != '/')
		throw ParseException("Location's root directive value must start with a slash '/", _configFileName, it->_posInLine, it->_line);
	if (!pathExists(_currentLocation.root))
		throw ParseException("Location's root directive value must exist", _configFileName, it->_posInLine, it->_line);
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's root directive");
}

void	ConfigParser::parseIndexDirective(std::vector<Token>::const_iterator &it)
{
	_currentLocation.indexes.clear();
	while (it != _tokens.end() && it->_type == T_WORD)
	{
		_currentLocation.indexes.push_back(it->_content);
		++it;
	}
	if (_currentLocation.indexes.empty())
		throw ParseException("Expected index file(s) after location's index directive", _configFileName, it->_posInLine, it->_line);
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's index directive");
}

void	ConfigParser::parseAllowMethodsDirective(std::vector<Token>::const_iterator &it)
{
	_currentLocation.allowedMethods.clear();
	while (it != _tokens.end() && it->_type == T_WORD)
	{
		if (it->_content == "GET" || it->_content == "POST" || it->_content == "DELETE")
			_currentLocation.allowedMethods.push_back(it->_content);
		else
			throw ParseException("Unknown method " + it->_content + " found in location's allowed_methods directive", _configFileName, it->_posInLine, it->_line);
		++it;
	}
	if (_currentLocation.allowedMethods.empty())
		throw ParseException("Expected method(s) after location's allowed_methods directive", _configFileName, it->_posInLine, it->_line);
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's allowed_methods directive");
}

void	ConfigParser::parseAutoIndexDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected value after location's autoindex directive", _configFileName, it->_posInLine, it->_line);
	if (it->_content == "on")
		_currentLocation.autoIndex = true;
	else if (it->_content == "off")
		_currentLocation.autoIndex = false;
	else
		throw ParseException("Expected on or off value after location's autoindex directive", _configFileName, it->_posInLine, it->_line);
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's autoindex directive");
}

void	ConfigParser::parseUploadEnableDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected value after location's upload_enable directive", _configFileName, it->_posInLine, it->_line);
	if (it->_content == "on")
		_currentLocation.uploadPerm = true;
	else if (it->_content == "off")
		_currentLocation.uploadPerm = false;
	else
		throw ParseException("Expected on or off value after location's upload_enable directive", _configFileName, it->_posInLine, it->_line);
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's upload_enable directive");
}

void	ConfigParser::parseUploadStoreDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected path after location's upload_path directive", _configFileName, it->_posInLine, it->_line);
	_currentLocation.uploadTo = it->_content;
	if (_currentLocation.uploadTo[0] != '/')
		throw ParseException("Location's upload_path directive value must start with a slash '/'", _configFileName, it->_posInLine, it->_line);
	if (!pathExists(_currentLocation.uploadTo))
		throw ParseException("Location's upload_path directive value must exist", _configFileName, it->_posInLine, it->_line);
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's upload_path directive");
}

void	ConfigParser::parseReturnDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || it->_type != T_NUMBER)
		throw ParseException("Expected status code after location's return directive", _configFileName, it->_posInLine, it->_line);
	int	statusCode = atoi(it->_content.c_str());
	if (statusCode < 300 || statusCode > 399)
		throw ParseException("Invalid status code: " + it->_content + " after location's return directive", _configFileName, it->_posInLine, it->_line);
	_currentLocation.redirectionCode = statusCode;
	++it;
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected path after location return directive's status code", _configFileName, it->_posInLine, it->_line);
	_currentLocation.redirectTo = it->_content;
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's return directive");
}

void	ConfigParser::parseCgiExtensionDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected cgi extension after location's cgi_extension directive", _configFileName, it->_posInLine, it->_line);
	std::string	ext = it->_content;
	if (ext[0] != '.')
		throw ParseException("CGI extension must start with a dot '.'", _configFileName, it->_posInLine, it->_line);
	++it;
	if (it == _tokens.end() || it->_type != T_WORD)
		throw ParseException("Expected interpreter's path after location cgi_extension directive's extension", _configFileName, it->_posInLine, it->_line);
	std::string	path = it->_content;
	if (path[0] != '/')
		throw ParseException("CGI interpreter's path must start with a slash '/'", _configFileName, it->_posInLine, it->_line);
	if (!pathExists(path))
		throw ParseException("CGI interpreter's path doesn't exist", _configFileName, it->_posInLine, it->_line);
	if (!isExecutable(path))
		throw ParseException("CGI interpreter's path isn't executable", _configFileName, it->_posInLine, it->_line);
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's cgi_extension directive");
	_currentLocation.cgiExtensions[ext] = path;
}

void	ConfigParser::parseLocationBodySizeDirective(std::vector<Token>::const_iterator &it)
{
	if (it == _tokens.end() || (it->_type != T_WORD && it->_type != T_NUMBER))
		throw ParseException("Expected size after location's client_max_body_size directive", _configFileName, it->_posInLine, it->_line);
	std::string bodySize = it->_content;
	if (it->_type == T_NUMBER)
		_currentLocation.clientMaxBodySize = atoi(bodySize.c_str());
	else if (it->_type == T_WORD)
		_currentLocation.clientMaxBodySize = convertSize(it, bodySize);
	++it;
	expectToken(it, T_SEMI_COLON, "Expected semi-colon ';' to conclude location's client_max_body_size directive");
}

void	ConfigParser::validateConfig(void)
{
	if (_config.configs.empty())
		throw ParseException("No server blocks found in configuration file", _configFileName, 0, 0);
	for (size_t i = 0; i < _config.configs.size(); i++)
	{
		if (_config.configs[i].listens.empty())
			throw ParseException("Server contains no listen directives", _configFileName, 0, 0);
		std::set<std::string>	locPaths;
		if (!pathExists(_config.configs[i].root))
				throw ParseException("Server root's path " + _config.configs[i].root + " doesn't exist", _configFileName, 0, 0);
		for (size_t j = 0; j < _config.configs[i].locations.size(); j++)
		{
			const std::string	&path = _config.configs[i].locations[j].path;
			if (locPaths.find(path) != locPaths.end())
				throw ParseException("Location path " + path + " is duplicated", _configFileName, 0, 0);
			if (path[0] != '/')
				throw ParseException("Location path " + path + " doesn't start with a slash '/'", _configFileName, 0, 0);
			if (path.size() > 1 && path[path.size() - 1] == '/')
				throw ParseException("Location path " + path + " shouldn't end with a slash '/'", _configFileName, 0, 0);
			locPaths.insert(path);
		}
		for (std::map<int, std::string>::iterator it = _config.configs[i].errorPages.begin(); it != _config.configs[i].errorPages.end(); ++it)
		{
			std::string	fullPath = _config.configs[i].root + it->second;
			if (!pathExists(fullPath))
				throw ParseException("Error page's path " + fullPath + " doesn't exist", _configFileName, 0, 0);
		}
	}
}

bool	ConfigParser::invalidHostIp(const std::string &ipAddr)
{
	struct in_addr	addr;
	return inet_pton(AF_INET, ipAddr.c_str(), &addr) != 1;
}

void	ConfigParser::expectToken(std::vector<Token>::const_iterator &it, TokenType type, std::string errorMsg)
{
	if (it == _tokens.end())
		throw ParseException("Unexpected end of file", _configFileName, it->_posInLine, it->_line);
	else if (it->_type != type)
		throw ParseException(errorMsg, _configFileName, it->_posInLine, it->_line);
	++it;
}

size_t	ConfigParser::convertSize(std::vector<Token>::const_iterator &it, const std::string &bodySize)
{
	std::string	sizePart;
	char		unitPart = '\0';
	for (size_t i = 0; i < bodySize.size(); i++)
	{
		if (isdigit(bodySize[i]))
			sizePart += bodySize[i];
		else
		{
			unitPart = bodySize[i];
			break;
		}
	}
	if (sizePart.empty())
		throw ParseException("Expected number in client_max_body_size's value", _configFileName, it->_posInLine, it->_line);
	size_t	value = static_cast<size_t>(atoi(sizePart.c_str()));
	switch (unitPart)
	{
		case 'K':
			value *= 1024;
			break;
		case 'M':
			value *= 1024 * 1024;
			break;
		case 'G':
			value *= 1024 * 1024 * 1024;
			break;
		default:
			throw ParseException("Unknown unit in client_max_body_size's value", _configFileName, it->_posInLine, it->_line);
	}
	return (value);
}

bool ConfigParser::pathExists(const std::string& path)
{
	struct stat	st;
	return stat(path.c_str(), &st) == 0;
}

bool ConfigParser::isExecutable(const std::string& path)
{
	return access(path.c_str(), X_OK) == 0;
}

int	ConfigParser::getServerCount() const
{
	return _serverCount;
}

std::vector<std::pair<int, std::pair<std::string, int> > >	&ConfigParser::getAllListens()
{
	return _allListens;
}
