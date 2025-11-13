/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 09:15:00 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/13 16:27:28 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sstream>

bool Server::setMode(Channel &channel, Client &client, t_message &message)
{
	std::string modeStr = message.params[1]; 
	bool addMode = true;
	size_t paramIndex = 2; // index de message.params
	
	for (size_t j = 0; j < modeStr.size(); j++)
	{
		if (modeStr[j] == '+')
			addMode = true;	
		else if (modeStr[j] == '-')
			addMode = false;
		else
		{
			if (modeStr[j] == 'i')
				channel._inviteOnly = addMode;
			else if (modeStr[j] == 't')
				channel._topicOperatorOnly = addMode;
			else if (modeStr[j] == 'k')
			{
				if (addMode)
				{
					if (paramIndex < message.params.size())
					{
						channel._key = message.params[paramIndex];
						paramIndex++;
					}
					else
					{
						sendClient(461, client, "Not enough parameters");
						return false;
					}
				}
				else
					channel._key = "";
			}
			else if (modeStr[j] == 'l')
			{
				if (addMode)
				{
					if (paramIndex < message.params.size())
					{
						std::stringstream ss(message.params[paramIndex]);
						int limit;
						ss >> limit;
						channel._userLimit = limit;
						paramIndex++;
					}
					else
					{
						sendClient(461, client, "Not enough parameters");
						return false;
					}
				}
				else
					channel._userLimit = -1;	
			}
			else if (modeStr[j] == 'o')
			{
				if (paramIndex < message.params.size())
				{				
					std::string nickToModify = message.params[paramIndex];
					paramIndex++;
					try
					{
						Client &clientToModify = getClientByNick(nickToModify);
						if (!channel.isMember(clientToModify._fd))
						{
							sendClient(441, client, nickToModify + " " + channel._name + " :They aren't on that channel");
							return false;
						}
        				if (addMode)
            				channel.addOperator(clientToModify._fd); 
        				else
            				channel.removeOperator(clientToModify._fd);
					}
					catch (const std::exception &e)
					{
						sendClient(401, client, nickToModify + " :No such nick");
						return false;
					}	
				}
				else
				{
					sendClient(461, client, "Not enough parameters");
					return false;
				}
			}
			else
			{
				sendClient(472, client, "Unknown mode char");
				return false;
			}
		}
	}
	return true;
}


void Server::mode(t_message &message, Client &client)
{
	if (message.params.empty())
		return sendClient(461, client, "Not enough parameters");
	
	std::string name = message.params[0];

	std::map<std::string, Channel>::iterator it = _channels.find(name);
	if (it == _channels.end())
		return sendClient(403, client, "No such channel");
	
	Channel &channel = it->second;
	if (!channel.isMember(client._fd))
		return sendClient(442, client, "You're not on that channel");

	if (message.params.size() == 1)
	{
		std::string modes = "+";
		if (channel._inviteOnly == true)
			modes += "i";
		if (channel._topicOperatorOnly == true)
			modes += "t";
		if (!channel._key.empty())
			modes += "k";
		if (channel._userLimit > 0)
			modes += "l";
		sendClient(324, client, name + " " + modes);
		return;
	}
	else if (message.params.size() > 1)
	{
		if (!channel.isOperator(client._fd))
			return sendClient(482, client, "You're not channel operator");
		bool success = setMode(channel, client, message);
		if (success)
		{
			std::string modeChange = message.params[1];
			std::string target;
			if(message.params.size() > 2)
				target = message.params[2];

			std::string reply = client._nick + ": MODE " + channel._name + " " + modeChange;
			if (!target.empty())
				reply += " " + target;
			sendInChannel(channel, -1, reply);
		}
		else
			return ;
	}
}