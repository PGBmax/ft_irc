/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:21:21 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/17 02:25:19 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Channel.hpp"
#include "Bot.hpp"
#include <string>
#include <poll.h>
#include <vector>
#include <map>

typedef struct s_message
{
	std::string command; 
	std::vector<std::string> params;
} 	t_message;

class Server
{
	public :
		Server(int port, const std::string &password);
		~Server();

		void run();

		class SignalHandler: public std::exception
		{
			public: virtual const char* what() const throw();
		};

		void botSendToChannel(const std::string &channel, const std::string &message);
		void botSendToClient(int client_fd, const std::string &message);
		std::string botGetClientNick(int client_fd);
		bool botIsClientInChannel(int client_fd, const std::string &channel);

	private :
		int								_port;
		std::string						_password;
		int								_listenFd;
		std::vector<pollfd>				_pfds;
		std::map<int, Client>			_clients;
		std::map<std::string, Channel> 	_channels;

		void setupListenSocket();
		void acceptNewClient();
		void readFromClient(int fd);
		void closeClient(int fd);
		size_t getPID(int fd) const;
		Client &getClient(int fd);
		Client &getClientByNick(std::string &nick);
		Channel &getChannel(std::string &name);
		Bot* _bot;
		void handleLine(int fd, const std::string &line);
		void sendInChannel(Channel &channel, int senderFd, const std::string &line);
		

		void sendClient(int code, Client &client, std::string message);
		void sendMessage(Client &client, std::string message, pollfd &pfd);
		bool isNickValid(const std::string &nick);
		
		

		void ping(t_message &message, Client &client);
		void pass(t_message &message, Client &client);
		bool nick(t_message &message, Client &client);
		bool user(t_message &message, Client &client);
		void userRegister(Client &client);
		bool join(t_message &message, Client &client);
		bool part(t_message &message, Client &client);
		bool privmsg(t_message &message, Client &client);
		void mode(t_message &message, Client &client);
		bool setMode(Channel &channel, Client &client, t_message &message);
		bool kick(t_message &message, Client &client);
		bool topic(t_message &message, Client &client);
		bool invite(t_message &message, Client &client);
};
