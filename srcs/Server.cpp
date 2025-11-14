/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:57:28 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/14 10:05:39 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <stdexcept>
#include <arpa/inet.h>
#include <cerrno>

static void set_nonblocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("failed to set O_NONBLOCK flag");
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
