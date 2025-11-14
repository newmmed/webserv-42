/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Token.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/09 23:10:27 by ouvled            #+#    #+#             */
/*   Updated: 2025/10/21 15:47:45 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKEN_HPP
# define TOKEN_HPP
# include <string>

enum	TokenType
{
	T_WORD,
	T_STRING,
	T_NUMBER,
	T_LEFT_BRACE,
	T_RIGHT_BRACE,
	T_SEMI_COLON,
	T_EOF
};

class	Token
{
public:
	std::string	_content;
	TokenType	_type;
	size_t		_posInLine;
	int			_line;
	Token();
	Token(std::string content, TokenType type, size_t posInLine, int line);
	const std::string	&getContent() const;
	TokenType			getType() const;
	size_t				getPosInLine() const;
	int					getLine() const;
};

#endif