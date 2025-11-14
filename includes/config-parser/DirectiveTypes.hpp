/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveTypes.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 16:04:17 by ouvled            #+#    #+#             */
/*   Updated: 2025/10/21 16:09:16 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVETYPES_HPP
# define DIRECTIVETYPES_HPP

# include <string>

enum ServerDirectives
{
	D_SERVER_LISTEN,
	D_SERVER_LOCATION,
	D_SERVER_ERROR_PAGE,
	D_SERVER_ROOT,
	D_SERVER_CLIENT_MAX_BODY_SIZE,
	D_SERVER_UNKNOWN
};

enum LocationDirectives
{
	D_LOC_ROOT,
	D_LOC_UPLOAD_STORE,
	D_LOC_ALLOW_METHODS,
	D_LOC_INDEX,
	D_LOC_UPLOAD_ENABLE,
	D_LOC_AUTOINDEX,
	D_LOC_RETURN,
	D_LOC_CLIENT_MAX_BODY_SIZE,
	D_LOC_CGI_EXTENSION,
	D_LOC_UNKNOWN
};

ServerDirectives	getServerDirective(const std::string &word);
LocationDirectives	getLocationDirective(const std::string &word);

#endif