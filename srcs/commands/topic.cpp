/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 09:15:03 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/13 16:29:43 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

bool Server::topic(t_message &message, Client &client)
{
	if (message.params.empty())
	{
		sendClient(461, client, "Not enough parameters");
		return false;
	}

	std::string channelName = message.params[0];

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

	if (message.params.size() == 1)
	{
		if (channel._topic.empty())
			sendClient(331, client, channelName + " No topic is set");
		else
			sendClient(332, client, channelName + " " + channel._topic);
		return true;
	}

	if (channel._topicOperatorOnly && !channel.isOperator(client._fd))
	{
		sendClient(482, client, "You're not channel operator");
		return false;
	}

	std::string newTopic = message.params[1];
	channel._topic = newTopic;

	std::string topicMessage = ":" + client._nick + " TOPIC " + channelName + " :" + newTopic;
	sendInChannel(channel, -1, topicMessage);

	return true;
}