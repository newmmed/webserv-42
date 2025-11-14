/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListeningSocket.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 19:16:34 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/05 20:17:46 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTENINGSOCKET_HPP
# define LISTENINGSOCKET_HPP

# include <string>
# include "../config-parser/Config.hpp"

typedef struct	ListeningSocket
{
	ServerConfig	_sconf;
	std::string		_host;
	int				_port;
	int				_fd;
	ListeningSocket();
	ListeningSocket(ServerConfig sconf, std::string host, int port, int fd);
}				ListeningSocket;


#endif