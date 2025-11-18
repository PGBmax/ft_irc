/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 16:52:58 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/17 02:46:21 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

bool Server::privmsg(t_message &message, Client &client)
{
    if (message.params.size() < 2)
	{
        sendClient(461, client, "PRIVMSG :Not enough parameters");
		return false;
	}

    std::string target = message.params[0];
    std::string text = message.params[1];
    std::string line = ":" + client._nick + " PRIVMSG " + target + " :" + text;

    if (target[0] == '#') 
	{
        if (_channels.find(target) == _channels.end())
		{
            sendClient(403, client, target + " :No such channel");
			return false;
		}

        Channel &channel = getChannel(target);
        if (!channel.isMember(client._fd))
		{
            sendClient(404, client, target + " :Cannot send to channel");
			return false;
		}

        sendInChannel(channel, client._fd, line);
        
        if (_bot)
            _bot->handleBotCommand(message, client, target);
    } 
	else
	{
		try
		{
			Client &dest = getClientByNick(target);
			sendMessage(dest, line, _pfds[getPID(dest._fd)]);
		}
		catch(...)
		{
			sendClient(401, client, target + " :No such nick");
			return false;
		}
    }
	return true;
}
