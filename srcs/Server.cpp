/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:57:28 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/13 17:01:32 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cerrno>
#include <algorithm>
#include <sstream>

static void set_nonblocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("failed to set O_NONBLOCK flag");
}

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

Server::Server(int port, const std::string &password) : _port(port), _password(password), _listenFd(-1)
{
	setupListenSocket();
}

Server::~Server()
{
	if (_listenFd != -1)
		close(_listenFd);
	std::cout << "Server destructed" << std::endl;
}

void Server::setupListenSocket()
{
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd == -1)
		throw std::runtime_error("listen socket creation failed");

	int turnOn = 1;
	if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &turnOn, sizeof(turnOn)) == -1)
		throw std::runtime_error("set option on listen socket SO_REUSEADDR failed");

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	if (bind(_listenFd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("bind listen socket to IPV4 address failed");

	if (listen(_listenFd, SOMAXCONN) == -1)
		throw std::runtime_error("listen failed");

	set_nonblocking(_listenFd);

	pollfd p;
	p.fd = _listenFd;
	p.events = POLLIN;
	p.revents = 0;
	_pfds.push_back(p);

	std::cout << "Socket listening on port " << _port << std::endl;
}

void Server::run()
{
	while (true)
	{
		int n = poll(&_pfds[0], _pfds.size(), -1);
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll failed");
		}

		if (_pfds[0].revents & POLLIN)
			acceptNewClient();

		for (size_t i = 1; i < _pfds.size(); ++i)
		{
			if (_pfds[i].revents & POLLIN)
				readFromClient(_pfds[i].fd);
			if (_pfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				closeClient(_pfds[i].fd);
				--i;
				continue;
			}
			if ((_pfds[i].revents & POLLOUT))
			{
				Client &client = getClient(_pfds[i].fd);
				if (!client._out.empty())
				{
					ssize_t sent = send(_pfds[i].fd, client._out.data(), client._out.size(), 0);
					if (sent > 0)
						client._out.erase(0, sent);
				}
				if (client._out.empty())
					_pfds[i].events &= ~POLLOUT;
			}
		}
	}
}

void Server:: acceptNewClient()
{
	sockaddr_in addr;
	socklen_t len = sizeof(addr);

	int fd = accept(_listenFd, (struct sockaddr*) &addr, &len);
	if (fd == -1)
		return;
	set_nonblocking(fd);

	pollfd p;
	p.fd = fd;
	p.events = POLLIN;
	p.revents = 0;
	_pfds.push_back(p);

	_clients.insert(std::map<int, Client>::value_type(fd, Client(fd)));

	char ip[64];
	const char *res = inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
	std::cout << "Client " << fd << " connected from " << (res ? ip : "?") << ":" << ntohs(addr.sin_port) << std::endl;
}

void Server::readFromClient(int fd)
{
	Client &client = getClient(fd);
	char buffer[4096];

	ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
	if (n <= 0)
	{
		if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
			return closeClient(fd);
		return;
	}
	
	client._in.append(buffer, n);
	
	size_t i;
	while ((i = client._in.find("\r\n")) != std::string::npos)
	{
		std::string line = client._in.substr(0, i);
		client._in.erase(0, i + 2);
		handleLine(fd, line);
		_pfds[getPID(fd)].events |= POLLOUT;
	}
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

void Server::closeClient(int fd)
{
	std::cout << "Client " << fd << " quit" << std::endl;

	close(fd);
	_clients.erase(fd);
	_pfds.erase(_pfds.begin() + getPID(fd));
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

const char* Server::SignalHandler::what() const throw()
{
	return (" Signal Detected ! Closing Server.\n");
}
