/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:57:28 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/05 11:30:55 by nolecler         ###   ########.fr       */
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

static void sendMessage(Client &client, std::string message, pollfd &pfd)
{
	client._out += message + "\r\n";
	pfd.events |= POLLOUT;
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
	std::set<int>::const_iterator it = channel._members.begin();
	for (; it != channel._members.end(); ++it)
	{
		if (*it == senderFd)
			continue;

		Client &client = getClient(*it);
		sendMessage(client, line, _pfds[getPID(*it)]);
	}
}

void Server::ping(t_message &message, Client &client)
{
	std::string param = message.params.empty() ? "" : message.params[0];
	client._out += "PONG: " + param + "\r\n";
	_pfds[getPID(client._fd)].events |= POLLOUT;
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

void Server::userRegister(Client &client)
{
	if (client._isPasswordValid && !client._nick.empty() && !client._user.empty())
	{
		client._registered = true;
		std::cout << "Client " << client._fd << " registered with name " << client._name << std::endl;
		return sendClient(001, client, "Welcome to ft_irc");
	}
}

bool Server::join(t_message &message, Client &client)
{
	if (message.params.empty())
	{
		sendClient(461, client, "Not enough parameters");
		return false;
	}

	std::string name = message.params[0];
	std::string key = (message.params.size() > 1 ? message.params[1] : "");
	
	if (name.empty() || name[0] != '#')
	{
		sendClient(403, client, "Invalid channel name");
		return false;
	}

	
	if (_channels.find(name) == _channels.end())
		_channels.insert(std::map<std::string, Channel>::value_type(name, Channel(name)));
	Channel &channel = getChannel(name);
	
	if (channel._inviteOnly && !channel._invited.count(client._fd))
	{
		sendClient(473, client, name + " Cannot join channel (+i)");
		return false;
	}

	if (!channel._key.empty() && channel._key != key)
	{
		sendClient(475, client, name + " Cannot join channel (+k)");
		return false;
	}

	if (channel._userLimit > 0 && static_cast<int>(channel._members.size()) >= channel._userLimit)
	{
		sendClient(471, client, name + " Cannot join channel (+l)");
		return false;
	}

	channel._members.insert(client._fd);

	if (channel._members.size() == 1)
		channel._operators.insert(client._fd);

	std::string line = ":" + client._nick + " JOIN " + name;
	sendInChannel(channel, -1, line);

	if (channel._topic.empty())
		sendClient(331, client, name + " No topic is set");
	else
		sendClient(332, client, name + " " + channel._topic);

	std::string nickList;
	std::set<int>::iterator it = channel._members.begin();
	for (; it != channel._members.end(); ++it)
	{
		Client &client = getClient(*it);
		nickList += client._nick + " ";
	}
	
	sendClient(353, client, name + " NAMES LIST: " + nickList);
	sendClient(366, client, name + " End of NAMES list");
	return true;
}

bool Server::part(t_message &message, Client &client)
{
	if (message.params.empty())
	{
		sendClient(461, client, "Not enough parameters");
		return false;
	}

	std::string name = message.params[0];
	std::map<std::string, Channel>::iterator it = _channels.find(name);
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

	std::string reason = message.params.size() > 1 ? message.params[1] : "Leaving";
	std::string line = client._nick + " PART " + name + " :" + reason;
	sendInChannel(channel, -1, line);

	channel._members.erase(client._fd);
	channel._operators.erase(client._fd);

	if (channel._members.empty())
		_channels.erase(it);
	return true;
}

bool Server::privmsg(t_message &message, Client &client)
{
    if (message.params.size() < 2)
	{
        sendClient(461, client, "Not enough parameters");
		return false;
	}

    std::string target = message.params[0];
    std::string text = message.params[1];
    std::string line = ":" + client._nick + " PRIVMSG " + target + " :" + text;

    if (target[0] == '#') 
	{
        if (_channels.find(target) == _channels.end())
		{
            sendClient(403, client, "No such channel");
			return false;
		}

        Channel &channel = getChannel(target);
        if (!channel.isMember(client._fd))
		{
            sendClient(404, client, "Cannot send to channel");
			return false;
		}

        sendInChannel(channel, client._fd, line);
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
			sendClient(401, client, "No such nick");
			return false;
		}
    }
	return true;
}


// Erreur							Code		Condition
// Pas assez de paramètres			461		Manque d’argument pour un mode (+k, +l, +o)
// Canal inexistant				    403		MODE sur un canal qui n’existe pas
// Non membre du canal				442		Le client n’est pas dans le canal
// Pas opérateur					482		Le client n’a pas les droits
// Nick inconnu ou hors du canal 	441		+o ou -o sur un nick inexistant

// 10   MODE #test +itk  A GERER
//      MODE #test +itk 1234 A GERER


// void Server::setMode(Channel &channel, Client &client, t_message &message)
// {
// 	std::string modeStr = message.params[1];
// 	bool addMode;
// 	char modeChar;
	
// 	//MODE #test +itk 1234
	
// 	if (modeStr[0] == '+')
// 		addMode = true;
// 	else
// 		addMode = false;
		
// 	modeChar = modeStr[1];
	
// 	if (modeChar == 'i')
// 		channel._inviteOnly = addMode;
// 	else if (modeChar == 't')
// 		channel._topicOperatorOnly = addMode;
// 	else if (modeChar == 'k')
// 	{
// 		//MODE #test +k 1234
// 		if (addMode && message.params.size() > 2)
// 			channel._key = message.params[2];
// 		// MODE #test -k
// 		else if (!addMode)
// 			channel._key = ""; // on enleve le mdp
// 	}
// 	else if (modeChar == 'l')
// 	{
// 		std::stringstream ss(message.params[2]);
// 		int limit;
// 		ss >> limit;
		
// 		// MODE #test +l 5	
// 		if (addMode && message.params.size() > 2)
// 			channel._userLimit = limit;
//     	else if (!addMode) // MODE #test -l
//         	channel._userLimit = -1;
// 	}
// 	else if (modeChar == 'o')
// 	{
// 		if (message.params.size() > 2)
//     	{
//         	std::string nickToModify = message.params[2];
// 			try
// 			{
// 				Client &clientToModify = getClientByNick(message.params[2]);
// 				if (!channel.isMember(clientToModify._fd))
// 				{
// 					sendClient(441, client, nickToModify + " " + channel._name + " :They aren't on that channel");
// 					return;
// 				}
//         		if (addMode)
//             		channel.addOperator(clientToModify._fd); // +o lili 
//         		else
//             		channel.removeOperator(clientToModify._fd); // -o lili
// 			}
// 			catch (const std::exception &e)
// 			{
// 				sendClient(401, client, nickToModify + " :No such nick");
// 				return;
// 			}
//     	}
// 		else
// 			sendClient(461, client, "Not enough parameters");
// 	}
// 	else
// 		sendClient(472, client, "Unknown mode char");
	
// }




void Server::setMode(Channel &channel, Client &client, t_message &message)
{
	std::string modeStr = message.params[1]; 
	bool addMode;
	int paramIndex = 2; // index de message.params
	
	//MODE #channel +itklo 1234 5 Alice
	//message.params[0] = "#channel"
	//message.params[1] = modeStr = "+itklo"
	//message.params[2] = "1234" // int paramIndex = 2
	//message.params[3] = "5"
	//message.params[4] = "Alice"
	
	if (modeStr[0] == '+')
		addMode = true;
	else
		addMode = false;
		
	for (int j = 1; j < modeStr.size(); j++)
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
					sendClient(461, client, "Not enough parameters");
			//MODE #test +kl 1234 5  --> A GERER 
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
					sendClient(461, client, "Not enough parameters");
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
						return;
					}
        			if (addMode)
            			channel.addOperator(clientToModify._fd); // +o lili 
        			else
            			channel.removeOperator(clientToModify._fd); // -o lili
				}
				catch (const std::exception &e)
				{
					sendClient(401, client, nickToModify + " :No such nick");
					return;
				}	
			}
			else
			{
				sendClient(461, client, "Not enough parameters");
			}
		}
		else
			sendClient(472, client, "Unknown mode char");
	}
}









void Server::announceModeChange(Channel &channel, Client &client, t_message &message)
{
	std::string modeChange = message.params[1];
    std::string targetNick;
    if (message.params.size() > 2)
        targetNick = message.params[2];

	std::string reply = client._nick + " MODE " + channel._name + " " + modeChange;
    if (!targetNick.empty())
        reply += " " + targetNick;
		
	std::set<int>::iterator it = channel._members.begin();
	for (; it != channel._members.end(); ++it)
	{
    	int memberFd = *it; //fd du membre
    	sendClient(0, _clients[memberFd], reply);
	}
}


void Server::mode(t_message &message, Client &client)
{
	// cas ou cmd = MODE 
	if (message.params.empty())
		return sendClient(461, client, "Not enough parameters");
	
	// cas ou cmd = MODE #channelName
	std::string name = message.params[0];

	// est ce que le channel existe
	std::map<std::string, Channel>::iterator it = _channels.find(name);
	if (it == _channels.end())
		return sendClient(403, client, "No such channel");
	
	// le channel existe : est ce que le demandeur est membre
	Channel &channel = it->second;
	if (!channel.isMember(client._fd))
		return sendClient(442, client, "You're not on that channel");

	// le demandeur est membre : quel est la commande exacte
	// si cmd = MODE #channel alors tous les membres ont droit a cette cmd
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
		// si cmd = MODE #channel +autre params
		// on verifie si le demandeur n'est pas l'operateur
		if (!channel.isOperator(client._fd))
			return sendClient(482, client, "You're not channel operator");
		// sinon si le demandeur est l'operateur
		setMode(channel, client, message); 
		//Annoncer le changement a tous les membres
		announceModeChange(channel, client, message);
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
