/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 09:14:57 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/13 16:49:41 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <algorithm>

bool Server::kick(t_message &message, Client &client)
{
	if (message.params.size() < 2)
	{
		sendClient(461, client, "Not enough parameters");
		return false;
	}

	std::string channelName = message.params[0];
	std::string targetNick = message.params[1];
	std::string reason = message.params.size() > 2 ? message.params[2] : client._nick;

	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it == _channels.end())
	{
		sendClient(403, client, "No such channel");
		return false;
	}

	Channel &channel = it->second;

	if (!channel.isMember(client._fd))
	{
		sendClient(442, client, "You're not on that channel");
		return false;
	}
	if (!channel.isOperator(client._fd))
	{
		sendClient(482, client, "You're not channel operator");
		return false;
	}

	Client *targetClient = NULL;
	try
	{
		targetClient = &getClientByNick(targetNick);
	}
	catch(...)
	{
		sendClient(401, client, "No such nick");
		return false;
	}

	if (!channel.isMember(targetClient->_fd))
	{
		sendClient(441, client, targetNick + " They aren't on that channel");
		return false;
	}

	if (targetClient->_fd == client._fd)
	{
		sendClient(484, client, "You cannot kick yourself from the channel");
		return false;
	}

	std::string kickMessage = ":" + client._nick + " KICK " + channelName + " " + targetNick + " :" + reason;
	sendInChannel(channel, -1, kickMessage);

	std::vector<int>::iterator iter = find(channel._members.begin(), channel._members.end(), targetClient->_fd);
	if (iter != channel._members.end())
		channel._members.erase(iter);
		
	channel._operators.erase(targetClient->_fd);
	channel._invited.erase(targetClient->_fd);

	if (channel._members.empty())
		_channels.erase(it);

	return true;
}



bool Server::part(t_message &message, Client &client)
{
	if (message.params.empty())
	{
		sendClient(461, client, "Not enough parameters");
		return false;
	}

	std::string name = message.params[0];
	std::map<std::string, Channel>::iterator it = _channels.find(name);
	if (it == _channels.end())
	{
		sendClient(403, client, "No such channel");
		return false;
	}

	Channel &channel = it->second;
	if (!channel.isMember(client._fd))
	{
		sendClient(442, client, "You're not on that channel");
		return false;
	}

	std::string reason = message.params.size() > 1 ? message.params[1] : "Leaving";
	std::string line = client._nick + " PART " + name + " :" + reason;
	sendInChannel(channel, -1, line);

	std::vector<int>::iterator iter = find(channel._members.begin(), channel._members.end(), client._fd);
	if (iter != channel._members.end())
		channel._members.erase(iter);
	channel._operators.erase(client._fd);
	if (channel._members.empty())
		_channels.erase(it);
	else if (!channel._members.empty() && channel._operators.empty())
		channel.addOperator(channel._members[0]);
	return true;
}