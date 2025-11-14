/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 23:29:38 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/13 11:02:52 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/server/Client.hpp"

Client::Client(int clientFd, std::string clientAddr, int clientPort, ListeningSocket &acceptingSocket)
	: fd(clientFd), addr(std::make_pair(clientAddr, clientPort)), acceptingSock(acceptingSocket),
	contentLength(0), bytesReceived(0), bytesSent(0), reqParsed(false), resSent(false), timeout(std::time(NULL)),
	headersParsed(false), requestComplete(false), keepAlive(false), expectedContentLength(0)
{
}
