/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectivesTypes.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouvled <ouvled@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 16:09:21 by ouvled            #+#    #+#             */
/*   Updated: 2025/11/04 18:56:54 by ouvled           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/config-parser/DirectiveTypes.hpp"

ServerDirectives	getServerDirective(const std::string &word)
{
	if (word == "listen") return D_SERVER_LISTEN;
	if (word == "root") return D_SERVER_ROOT;
	if (word == "error_page") return D_SERVER_ERROR_PAGE;
	if (word == "client_max_body_size") return D_SERVER_CLIENT_MAX_BODY_SIZE;
	if (word == "location") return D_SERVER_LOCATION;
	return D_SERVER_UNKNOWN;
}

LocationDirectives	getLocationDirective(const std::string &word)
{
	if (word == "root") return D_LOC_ROOT;
	if (word == "index") return D_LOC_INDEX;
	if (word == "allow_methods") return D_LOC_ALLOW_METHODS;
	if (word == "autoindex") return D_LOC_AUTOINDEX;
	if (word == "upload_enable") return D_LOC_UPLOAD_ENABLE;
	if (word == "upload_path") return D_LOC_UPLOAD_STORE;
	if (word == "return") return D_LOC_RETURN;
	if (word == "cgi_extension") return D_LOC_CGI_EXTENSION;
	if (word == "client_max_body_size") return D_LOC_CLIENT_MAX_BODY_SIZE;
	return D_LOC_UNKNOWN;
}
