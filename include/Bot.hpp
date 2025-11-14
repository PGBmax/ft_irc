/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 22:50:41 by pboucher          #+#    #+#             */
/*   Updated: 2025/11/14 15:14:01 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#pragma once

#include <map>
#include <string>
#include <vector>
#include "Connect4.hpp"

class Client;
class Server;
struct s_message;

class Bot
{
	public:
		Bot(Server* server);
		~Bot();
		
		bool handleBotCommand(struct s_message &message, Client &client, const std::string &channel);
		
		bool startConnect4(struct s_message &message, Client &client, const std::string &channel);
		bool acceptConnect4(Client &client, const std::string &channel);
		bool playConnect4(struct s_message &message, Client &client, const std::string &channel);
		bool forfeitConnect4(Client &client, const std::string &channel);
		
		void endGame(const std::string &gameId, const std::string &reason);
		
		void onClientDisconnected(int client_fd);
		void onPlayerLeftChannel(int client_fd, const std::string &channel);
		
	private:
		Server* _server;
		std::map<std::string, Connect4> _games;
		
		bool dropPiece(Connect4 &game, int column);
		bool checkWin(const Connect4 &game, int player);
		bool isBoardFull(const Connect4 &game);
		void displayBoard(const Connect4 &game, const std::string &channel);
		int botMove(const Connect4 &game);
		std::string generateGameId(const std::string &channel, int player1_fd, int player2_fd);
		
		void sendToChannel(const std::string &channel, const std::string &message);
		void sendToClient(int client_fd, const std::string &message);
		std::string getClientNick(int client_fd);
		bool isClientInChannel(int client_fd, const std::string &channel);
};