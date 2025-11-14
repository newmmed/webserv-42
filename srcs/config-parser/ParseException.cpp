/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseException.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 18:53:59 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/04 18:55:57 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/config-parser/ParseException.hpp"

ParseException::ParseException(const std::string &errMsg, const std::string &fileName, size_t posInLine, int line)
		: _errMsg(errMsg), _fileName(fileName), _posInLine(posInLine), _line(line)
{
	
}

const char	*ParseException::what() const throw()
{
	static std::string	fullMessage;
	std::ostringstream	oss;
	if (_line > 0)
		oss << "webserv: " << _errMsg << " in file: " << _fileName << ":" << _line << ":" << _posInLine;
	else
		oss << "webserv: " << _errMsg << " in file: " << _fileName;
	fullMessage = oss.str();
	return fullMessage.c_str();
}
