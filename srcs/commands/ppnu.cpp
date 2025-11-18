/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ppnu.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 16:55:52 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/13 16:56:33 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


void Server::ping(t_message &message, Client &client)
{
	std::string param = message.params.empty() ? "" : message.params[0];
	client._out += "PONG: " + param + "\r\n";
	_pfds[getPID(client._fd)].events |= POLLOUT;
}

void Server::pass(t_message &message, Client &client)
{
	if (message.params.empty())
		return sendClient(461, client, "PASS :Not enough parameters");
	if (client._registered)
		return sendClient(462, client, ":You may not reregister");

	client._isPasswordValid = (message.params[0] == _password);
	if (!client._isPasswordValid)
		return sendClient(464, client, ":Password incorrect");
}

bool Server::nick(t_message &message, Client &client)
{
	if (message.params.empty())
	{
		sendClient(431, client, ":No nickname given");
		return false;
	}
	
	std::string newNick = message.params[0];
	if (!isNickValid(newNick))
	{
		sendClient(432, client, ":Invalid nickname");
		return false;
	}
	std::map<int, Client>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it)
	{
		if (it->second._fd != client._fd && it->second._nick == newNick)
		{
			sendClient(433, client, newNick + " :Nickname is already taken");
			return false;
		}
	}
	client._nick = newNick;
	return true;
}

bool Server::user(t_message &message, Client &client)
{
	if (message.params.size() < 4)
	{
		sendClient(461, client, "USER :Not enough parameters");
		return false;
	}
	client._user = message.params[0];
	client._name = message.params[3];
	return true;
}
