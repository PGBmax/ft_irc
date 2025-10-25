/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rraumain <rraumain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:57:28 by nolecler          #+#    #+#             */
/*   Updated: 2025/10/25 18:25:30 by rraumain         ###   ########.fr       */
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

static void sendClient(int code, Client &client, std::string message)
{
	std::stringstream ss;

	ss << code << " " << (client._nick.empty() ? "*" : client._nick) << ": " << message << "\r\n";
	client._out += ss.str();
}

static bool isNickValid(const std::string &nick)
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
				readFromClient(i);
			if (_pfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				closeClient(i);
				--i;
				continue;
			}
			if ((_pfds[i].revents & POLLOUT))
			{
				Client &client = getClient(i);
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

void Server::readFromClient(size_t id)
{
	Client &client = getClient(id);
	char buffer[4096];

	ssize_t n = recv(client._fd, buffer, sizeof(buffer), 0);
	if (n <= 0)
	{
		if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
			return closeClient(id);
		return;
	}
	
	client._in.append(buffer, n);
	
	size_t i;
	while ((i = client._in.find("\r\n")) != std::string::npos)
	{
		std::string line = client._in.substr(0, i);
		client._in.erase(0, i + 2);
		handleLine(id, line);
		_pfds[id].events |= POLLOUT;
	}
}

void Server::ping(t_message &message, Client &client, size_t id)
{
	std::string param = message.params.empty() ? "" : message.params[0];
	client._out += "PONG: " + param + "\r\n";
	_pfds[id].events |= POLLOUT;
}

void Server::pass(t_message &message, Client &client)
{
	if (message.params.empty())
		return sendClient(461, client, "Not enough parameters");
	if (client._registered)
		return sendClient(462, client, "You may not reregister");

	client._isPasswordValid = (message.params[0] == _password);
	if (!client._isPasswordValid)
		return sendClient(464, client, "Password incorrect");
}

bool Server::nick(t_message &message, Client &client)
{
	if (message.params.empty())
	{
		sendClient(431, client, "No nickname given");
		return false;
	}
	
	std::string newNick = message.params[0];
	if (!isNickValid(newNick))
	{
		sendClient(432, client, "Invalid nickname");
		return false;
	}
	std::map<int, Client>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it)
	{
		if (it->second._fd != client._fd && it->second._nick == newNick)
		{
			sendClient(433, client, "Nickname is already taken");
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
		sendClient(461, client, "Not enough parameters");
		return false;
	}
	client._user = message.params[0];
	client._name = message.params[3];
	return true;
}

void Server::userRegister(t_message &message, Client &client)
{
	if (client._isPasswordValid && !client._nick.empty() && !client._user.empty())
	{
		client._registered = true;
		std::cout << "Client " << client._fd << " registered with name " << client._name << std::endl;
		return sendClient(001, client, "Welcome to ft_irc");
	}
}

void Server::handleLine(size_t id, const std::string &line)
{
	Client &client = getClient(id);
	t_message message = parseLine(line);

	if (message.command == "PING")
		return ping(message, client, id);

	if (message.command == "PASS")
		return pass(message, client);

	if (message.command == "NICK" && !nick(message, client))
		return;

	if (message.command == "USER" && !user(message, client))
		return;
	
	if (!client._registered)
		return userRegister(message, client);
}

void Server::closeClient(size_t id)
{
	int fd = _pfds[id].fd;
	std::cout << "Client " << fd << " quit" << std::endl;

	close(fd);
	_clients.erase(fd);
	_pfds.erase(_pfds.begin() + id);
}

Client &Server::getClient(size_t id)
{
	if (id == 0 || id >= _pfds.size())
		throw std::out_of_range("bad id");

	int fd = _pfds[id].fd;
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		throw std::runtime_error("client not found");
	return (it->second);
}
