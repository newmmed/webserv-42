/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/12 20:55:51 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/04 18:57:05 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/config-parser/Tokenizer.hpp"

Tokenizer::Tokenizer()
{
	
}

Tokenizer::Tokenizer(char *config_path) : configPath(config_path), pos(0), posInLine(1), line(1)
{
	std::ifstream	file;

	file.open(config_path);
	if (!file.is_open())
		throw	std::runtime_error("Error: cannot open config file");
	this->config = readConfig(file);
	file.close();
}

std::string	readConfig(std::ifstream &file)
{
	std::string	ret;
	std::string	line;

	while (std::getline(file, line))
		ret += line + '\n';
	return (ret);
}

Token	Tokenizer::getNextToken()
{
	skipWhiteSpaces();
	if (this->pos >= config.length())
		return Token("", T_EOF, this->posInLine, this->line);
	char	c = config[pos];
	if (c == '{')
	{
		posInLine++;
		pos++;
		return Token("{", T_LEFT_BRACE, this->posInLine, this->line);
	}
	if (c == '}')
	{
		posInLine++;
		pos++;
		return Token("}", T_RIGHT_BRACE, this->posInLine, this->line);
	}
	if (c == ';')
	{
		posInLine++;
		pos++;
		return Token(";", T_SEMI_COLON, this->posInLine, this->line);
	}
	if (c == '"')
		return parseString();
	if (isdigit(c) && isValidDigit()) // Need a function to make sure Token is not a word here
		return parseNumber();
	if (isdigit(c) || isalpha(c) || c == '_' || c == '/' || c == '.' || c == ':')
		return parseWord();
	throw ParseException("Unexpected character", configPath, posInLine, line);
}

bool	Tokenizer::isValidDigit()
{
	size_t	_pos = pos;

	while (config[_pos] != ' ' && config[_pos] != ';' && _pos < config.length())
	{
		if (!isdigit(config[_pos]))
			return false;
		_pos++;
	}
	return true;
}

void	Tokenizer::skipWhiteSpaces()
{
	while (pos < config.length())
	{
		if (config[pos] == '\n')
		{
			pos++;
			posInLine = 1;
			line++;
		}
		else if (config[pos] == '#')
		{
			while (pos < config.length() && config[pos] != '\n')
				pos++;
			posInLine = 1;
			// line++;
		}
		else if (isspace(config[pos]))
		{
			pos++;
			posInLine++;
		}
		else
			break ;
	}
}

Token	Tokenizer::parseString()
{
	pos++;
	posInLine++;
	size_t		start = pos;
	while (pos < config.length() && config[pos] != '"')
	{
		if (config[pos] == '\\')
		{
			pos++;
			posInLine++;
		}
		pos++;
		posInLine++;
	}
	if (pos >= config.length())
		throw ParseException("Unterminated string", configPath, posInLine, line);
	std::string	content = config.substr(start, pos - start);
	pos++;
	posInLine++;
	return Token(content, T_STRING, posInLine - (pos - start), line);
}

Token	Tokenizer::parseNumber()
{
	size_t	start = pos;
	while (pos < config.length() && isdigit(config[pos]))
	{
		pos++;
		posInLine++;
	}
	return Token(config.substr(start, pos - start), T_NUMBER, posInLine - (pos - start), line);
}

Token	Tokenizer::parseWord()
{
	size_t	start = pos;
	while (pos < config.length() && (isdigit(config[pos]) || isalpha(config[pos]) || config[pos] == '_' || config[pos] == '-' || config[pos] == '.' || config[pos] == ':' || config[pos] == '/' || isdigit(config[pos])))
	{
		pos++;
		posInLine++;
	}
	return Token(config.substr(start, pos - start), T_WORD, posInLine - (pos - start), line);
}

std::string	&Tokenizer::getConfig()
{
	return this->config;
}
