/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListeningSocket.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 19:53:31 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/10 00:39:08 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/server/ListeningSocket.hpp"

ListeningSocket::ListeningSocket(ServerConfig sconf, std::string host, int port, int fd) : _sconf(sconf), _host(host), _port(port), _fd(fd)
{
	
}
