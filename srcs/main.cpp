/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 10:31:14 by ouel-bou          #+#    #+#             */
/*   Updated: 2025/11/10 00:39:21 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/config-parser/ConfigParser.hpp"
#include <iostream>
#include "../includes/server/Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: webserv <config_file>" << std::endl;
		return 1;
	}
	// try
	// {
		ConfigParser	parser = ConfigParser(argv[1]);
		Config			servConfig = parser.parseConfig();
		(void)servConfig;
		Server	serv = Server(servConfig, parser.getAllListens(), parser.getServerCount());
		serv.startServer();
	// }
	// catch (const ParseException &e)
	// {
	// 	std::cerr << e.what() << std::endl;
	// 	return 1;
	// }
	// catch (const std::exception &e)
	// {
	// 	std::cerr << "webserv: " << e.what() << std::endl;
	// 	return 1;
	// }
}
