/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/14 09:10:04 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/14 10:05:22 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <algorithm>

bool Server::isNickValid(const std::string &nick)
{
	if (nick.empty())
		return false;
	for (size_t i = 0; i < nick.size(); ++i)
	{
		unsigned char c = nick[i];
		if (!(std::isalnum(c) || c == '-' || c == '_'))
			return false;
	}
	return true;
}

size_t Server::getPID(int fd) const
{
	size_t i = 1;
	for (; i < _pfds.size(); ++i)
	{
		if (_pfds[i].fd == fd)
			return i;
	}
	throw std::out_of_range("bad id");
}

bool Server::findPollIndex(int fd, size_t &index) const
{
	for (size_t i = 0; i < _pfds.size(); ++i)
	{
		if (_pfds[i].fd == fd)
		{
			index = i;
			return true;
		}
	}
	return false;
}

Client &Server::getClient(int fd)
{
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		throw std::runtime_error("client not found");
	return (it->second);
}

Client &Server::getClientByNick(std::string &nick)
{
	std::map<int, Client>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it)
	{
		if (it->second._nick == nick)
			return it->second;
	}
	throw std::runtime_error("client not found");
}

Channel &Server::getChannel(std::string &name)
{
	std::map<std::string, Channel>::iterator it = _channels.find(name);
	if (it == _channels.end())
		throw std::runtime_error("channel not found");
	return (it->second);
}

void Server::closeClient(int fd)
{
	std::map<int, Client>::iterator clientIt = _clients.find(fd);
	if (clientIt == _clients.end())
		return;

	Client &client = clientIt->second;
	std::string nick = client._nick.empty() ? "*" : client._nick;
	std::string quitMessage = ":" + nick + " QUIT :Client disconnected";

	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end();)
	{
		Channel &channel = it->second;
		bool wasMember = false;

		std::vector<int>::iterator memberIt = std::find(channel._members.begin(), channel._members.end(), fd);
		if (memberIt != channel._members.end())
		{
			channel._members.erase(memberIt);
			wasMember = true;
		}

		channel._operators.erase(fd);
		channel._invited.erase(fd);

		if (wasMember)
		{
			sendInChannel(channel, fd, quitMessage);
			if (channel._members.empty())
			{
				_channels.erase(it++);
				continue;
			}
			if (channel._operators.empty())
				channel.addOperator(channel._members[0]);
			if (_bot)
				_bot->onPlayerLeftChannel(fd, channel._name);
		}

		if (!wasMember && channel._members.empty())
		{
			_channels.erase(it++);
			continue;
		}
		else
			++it;
	}

	if (_bot)
		_bot->onClientDisconnected(fd);

	close(fd);
	size_t idx;
	if (findPollIndex(fd, idx))
		_pfds.erase(_pfds.begin() + idx);
	_clients.erase(clientIt);

	std::cout << "Client " << fd << " quit" << std::endl;
}
