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
	std::cout << "Client " << fd << " quit" << std::endl;

	close(fd);
	_clients.erase(fd);
	_pfds.erase(_pfds.begin() + getPID(fd));
}

