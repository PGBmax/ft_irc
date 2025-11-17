/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/14 09:19:13 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/14 09:46:21 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <algorithm>

static t_message parseLine(const std::string &line)
{
	t_message message;
	std::string tmp = line;
	size_t i = 0;

	if (!tmp.empty() && tmp[0] == ':')
	{
		size_t space = tmp.find(' ');
		if (space != std::string::npos)
			tmp.erase(0, space + 1);
		else
			tmp.clear();
	}

	while (i < tmp.size() && tmp[i] == ' ')
		++i;
	size_t start = i;
	while (i < tmp.size() && tmp[i] != ' ')
		++i;

	message.command = tmp.substr(start, i - start);
	std::transform(message.command.begin(), message.command.end(), message.command.begin(), toupper);
	
	while (i < tmp.size())
	{
		while (i < tmp.size() && tmp[i] == ' ')
			++i;
		if (i >= tmp.size())
			break;
		if (tmp[i] == ':')
		{
			message.params.push_back(tmp.substr(i + 1));
			break;
		}
		start = i;
		while (i < tmp.size() && tmp[i] != ' ')
			++i;
		message.params.push_back(tmp.substr(start, i - start));
	}
	return message;
}

void Server::handleLine(int fd, const std::string &line)
{
	Client &client = getClient(fd);
	t_message message = parseLine(line);

	if (message.command == "PING")
		return ping(message, client);

	if (message.command == "PASS")
		return pass(message, client);

	if (message.command == "NICK" && !nick(message, client))
		return;

	if (message.command == "USER" && !user(message, client))
		return;
	
	if (!client._registered)
		return userRegister(client);

	if (message.command == "JOIN" && !join(message, client))
		return;

	if (message.command == "PART" && !part(message, client))
		return;
	
	if (message.command == "PRIVMSG" && !privmsg(message, client))
		return;
	
	if (message.command == "KICK" && !kick(message, client))
		return;
	
	if (message.command == "TOPIC" && !topic(message, client))
		return;
	
	if (message.command == "INVITE" && !invite(message, client))
		return;
	
	if (message.command == "MODE")
		return mode(message, client);
}
