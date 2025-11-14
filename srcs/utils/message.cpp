/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/14 09:25:58 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/14 09:47:19 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sstream>
#include <iostream>

void Server::sendClient(int code, Client &client, std::string message)
{
	std::stringstream ss;

	ss << code << " " << (client._nick.empty() ? "*" : client._nick) << ": " << message << "\r\n";
	client._out += ss.str();
}

void Server::sendMessage(Client &client, std::string message, pollfd &pfd)
{
	client._out += message + "\r\n";
	pfd.events |= POLLOUT;
}

void Server::sendInChannel(Channel &channel, int senderFd, const std::string &line)
{	
	std::vector<int>::const_iterator it = channel._members.begin();
	for (; it != channel._members.end(); ++it)
	{
		if (*it == senderFd)
			continue;

		Client &client = getClient(*it);
		sendMessage(client, line, _pfds[getPID(*it)]);
	}
}

void Server::userRegister(Client &client)
{
	if (client._isPasswordValid && !client._nick.empty() && !client._user.empty())
	{
		client._registered = true;
		std::cout << "Client " << client._fd << " registered with name " << client._name << std::endl;
		return sendClient(001, client, "Welcome to ft_irc");
	}
}

const char* Server::SignalHandler::what() const throw()
{
	return (" Signal Detected ! Closing Server.\n");
}
