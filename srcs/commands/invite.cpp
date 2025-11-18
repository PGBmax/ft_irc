/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 09:14:53 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/13 16:21:18 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

bool Server::invite(t_message &message, Client &client)
{
	if (message.params.size() < 2)
	{
		sendClient(461, client, "INVITE :Not enough parameters");
		return false;
	}

	std::string targetNick = message.params[0];
	std::string channelName = message.params[1];

	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it == _channels.end())
	{
		sendClient(403, client, channelName + " :No such channel");
		return false;
	}

	Channel &channel = it->second;

	if (!channel.isMember(client._fd))
	{
		sendClient(442, client, channelName + " :You're not on that channel");
		return false;
	}

	if (channel._inviteOnly && !channel.isOperator(client._fd))
	{
		sendClient(482, client, channelName + " :You're not channel operator");
		return false;
	}

	Client *targetClient = NULL;
	try
	{
		targetClient = &getClientByNick(targetNick);
	}
	catch(...)
	{
		sendClient(401, client, targetNick + " :No such nick");
		return false;
	}

	if (channel.isMember(targetClient->_fd))
	{
		sendClient(443, client, targetNick + " :is already on channel");
		return false;
	}

	channel._invited.insert(targetClient->_fd);

	sendClient(341, client, targetNick + " " + channelName);
	
	std::string inviteMessage = ":" + client._nick + " INVITE " + targetNick + " " + channelName;
	sendMessage(*targetClient, inviteMessage, _pfds[getPID(targetClient->_fd)]);

	return true;
}
