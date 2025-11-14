/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseException.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/12 23:30:41 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/04 18:56:04 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSEEXCEPTION_HPP
# define PARSEEXCEPTION_HPP

#include <sstream>

class ParseException : public std::exception
{
private:
	std::string	_errMsg;
	std::string	_fileName;
	size_t		_posInLine;
	int			_line;
public:
	ParseException(const std::string &errMsg, const std::string &fileName, size_t posInLine, int line);
	virtual ~ParseException() throw() {}
	virtual const char	*what() const throw();
};

#endif