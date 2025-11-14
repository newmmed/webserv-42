/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/12 20:52:32 by ouvled            #+#    #+#             */
/*   Updated: 2025/10/14 01:01:53 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKENIZER_HPP
# define TOKENIZER_HPP
# include <string>
# include <fstream>
# include "Token.hpp"
# include "ParseException.hpp"

class Tokenizer
{
private:
	std::string	config;
	std::string	configPath;
	size_t		pos;
	size_t		posInLine;
	int			line;
public:
	Tokenizer();
	Tokenizer(char *config_path);
	Token		getNextToken();
	std::string	&getConfig();
private:
	bool		isValidDigit();
	void		skipWhiteSpaces();
	Token		parseString();
	Token		parseNumber();
	Token		parseWord();
};

std::string	readConfig(std::ifstream &file);

#endif