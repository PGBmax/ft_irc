/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 22:41:28 by pboucher          #+#    #+#             */
/*   Updated: 2025/11/06 23:41:44 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Bot.hpp"
#include "Client.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

static std::string intToString(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

Bot::Bot(Server* server) : _server(server) {}

Bot::~Bot() {}

bool Bot::handleBotCommand(t_message &message, Client &client, const std::string &channel)
{
	if (message.params.size() < 2)
		return false;
	
	std::string botMessage = message.params[1];
	
	std::istringstream iss(botMessage);
	std::string command;
	iss >> command;
	
	std::transform(command.begin(), command.end(), command.begin(), ::toupper);
	
	if (command == "CONNECT4")
		return startConnect4(message, client, channel);
	else if (command == "PLAY")
		return playConnect4(message, client, channel);
	
	return false;
}

bool Bot::startConnect4(t_message &message, Client &client, const std::string &channel)
{
	std::istringstream iss(message.params[1]);
	std::string command, targetPlayer;
	iss >> command >> targetPlayer;
	
	for (std::map<std::string, Connect4>::iterator it = _games.begin(); it != _games.end(); ++it)
	{
		if (it->second.getChannel() == channel && 
			(it->second.getPlayer1Fd() == client._fd || it->second.getPlayer2Fd() == client._fd) &&
			it->second.getState() != GAME_OVER)
		{
			sendToChannel(channel, "[ERROR] " + getClientNick(client._fd) + " is already in a game!");
			return true;
		}
	}
	
	if (targetPlayer.empty())
	{
		std::string gameId = generateGameId(channel, client._fd, -1);
		Connect4 &game = _games[gameId];
		game.setPlayer1Fd(client._fd);
		game.setPlayer2Fd(-1);
		game.setVsBot(true);
		game.setChannel(channel);
		game.setState(IN_GAME);
		srand(time(NULL));
		game.setCurrentPlayer((rand() % 2) + 1);
		
		sendToChannel(channel, "[GAME] " + getClientNick(client._fd) + " started a Connect 4 game against the bot!");
		displayBoard(game, channel);
		
		if (game.getCurrentPlayer() == 1)
			sendToChannel(channel, "[RED] " + getClientNick(client._fd) + "'s turn! Use 'PLAY <column>' (1-7)");
		else 
		{
			sendToChannel(channel, "[YELLOW] Bot's turn!");
			int botColumn = botMove(game);
			if (dropPiece(game, botColumn))
			{
				sendToChannel(channel, "[YELLOW] Bot plays column " + intToString(botColumn + 1));
				displayBoard(game, channel);
				
				if (checkWin(game, 2))
				{
					sendToChannel(channel, "[WIN] Bot wins! Game over!");
					_games.erase(gameId);
					return true;
				}
				
				game.setCurrentPlayer(1);
				sendToChannel(channel, "[RED] " + getClientNick(client._fd) + "'s turn! Use 'PLAY <column>' (1-7)");
			}
		}
		
		return true;
	}
	else
	{
		sendToChannel(channel, "[ERROR] Player vs player not implemented yet! Use: CONNECT4 (without target)");
		return true;
	}
}

bool Bot::playConnect4(t_message &message, Client &client, const std::string &channel)
{
	std::istringstream iss(message.params[1]);
	std::string command;
	int column;
	iss >> command >> column;
	
	if (column < 1 || column > 7)
	{
		sendToChannel(channel, "[ERROR] Invalid column! Use 1-7.");
		return true;
	}
	
	column--;
	
	Connect4 *activeGame = NULL;
	std::string activeGameId;
	
	for (std::map<std::string, Connect4>::iterator it = _games.begin(); it != _games.end(); ++it)
	{
		if (it->second.getChannel() == channel && 
			it->second.getState() == IN_GAME &&
			(it->second.getPlayer1Fd() == client._fd || it->second.getPlayer2Fd() == client._fd))
			{
			activeGame = &it->second;
			activeGameId = it->first;
			break;
		}
	}
	
	if (!activeGame)
	{
		sendToChannel(channel, "[ERROR] You're not in an active game!");
		return true;
	}
	
	bool isPlayerTurn = false;
	if (activeGame->getCurrentPlayer() == 1 && activeGame->getPlayer1Fd() == client._fd)
		isPlayerTurn = true;
	else if (activeGame->getCurrentPlayer() == 2 && activeGame->getPlayer2Fd() == client._fd)
		isPlayerTurn = true;	

	if (!isPlayerTurn)
	{
		sendToChannel(channel, "[ERROR] It's not your turn!");
		return true;
	}
	
	if (!dropPiece(*activeGame, column))
	{
		sendToChannel(channel, "[ERROR] Column " + intToString(column + 1) + " is full!");
		return true;
	}

	std::string playerSymbol = (activeGame->getCurrentPlayer() == 1) ? "[RED]" : "[YELLOW]";
	sendToChannel(channel, playerSymbol + " " + getClientNick(client._fd) + " plays column " + intToString(column + 1));
	
	displayBoard(*activeGame, channel);
	
	if (checkWin(*activeGame, activeGame->getCurrentPlayer()))
	{
		sendToChannel(channel, "[WIN] " + getClientNick(client._fd) + " wins! Game over!");
		_games.erase(activeGameId);
		return true;
	}
	
	activeGame->setCurrentPlayer((activeGame->getCurrentPlayer() == 1) ? 2 : 1);
	
	if (activeGame->isVsBot() && activeGame->getCurrentPlayer() == 2)
	{
		sendToChannel(channel, "[YELLOW] Bot's turn!");
		
		int botColumn = botMove(*activeGame);
		if (dropPiece(*activeGame, botColumn))
		{
			sendToChannel(channel, "[YELLOW] Bot plays column " + intToString(botColumn + 1));
			displayBoard(*activeGame, channel);
			
			if (checkWin(*activeGame, 2))
			{
				sendToChannel(channel, "[WIN] Bot wins! Game over!");
				_games.erase(activeGameId);
				return true;
			}
			
			activeGame->setCurrentPlayer(1);
			sendToChannel(channel, "[RED] " + getClientNick(activeGame->getPlayer1Fd()) + "'s turn! Use 'PLAY <column>' (1-7)");
		}
	}
	else
	{
		int nextPlayerFd = (activeGame->getCurrentPlayer() == 1) ? activeGame->getPlayer1Fd() : activeGame->getPlayer2Fd();
		std::string nextSymbol = (activeGame->getCurrentPlayer() == 1) ? "[RED]" : "[YELLOW]";
		sendToChannel(channel, nextSymbol + " " + getClientNick(nextPlayerFd) + "'s turn! Use 'PLAY <column>' (1-7)");
	}
	
	return true;
}

bool Bot::dropPiece(Connect4 &game, int column)
{
	if (column < 0 || column >= 7)
		return false;
	
	for (int row = 5; row >= 0; row--)
	{
		if (game.getBoard(row, column) == 0)
		{
			game.setBoard(row, column, game.getCurrentPlayer());
			return true;
		}
	}
	
	return false;
}

bool Bot::checkWin(const Connect4 &game, int player)
{
	for (int row = 0; row < 6; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			if (game.getBoard(row, col) == player &&
				game.getBoard(row, col + 1) == player &&
				game.getBoard(row, col + 2) == player &&
				game.getBoard(row, col + 3) == player)
				return true;
		}
	}
	
	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 7; col++)
		{
			if (game.getBoard(row, col) == player &&
				game.getBoard(row + 1, col) == player &&
				game.getBoard(row + 2, col) == player &&
				game.getBoard(row + 3, col) == player)
				return true;
		}
	}
	
	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			if (game.getBoard(row, col) == player &&
				game.getBoard(row + 1, col + 1) == player &&
				game.getBoard(row + 2, col + 2) == player &&
				game.getBoard(row + 3, col + 3) == player)
				return true;
		}
	}
	
	for (int row = 0; row < 3; row++)
	{
		for (int col = 3; col < 7; col++)
		{
			if (game.getBoard(row, col) == player &&
				game.getBoard(row + 1, col - 1) == player &&
				game.getBoard(row + 2, col - 2) == player &&
				game.getBoard(row + 3, col - 3) == player)
				return true;
		}
	}
	
	return false;
}

void Bot::displayBoard(const Connect4 &game, const std::string &channel)
{
	sendToChannel(channel, "  1 2 3 4 5 6 7 ");
	sendToChannel(channel, "+---------------+");
	
	for (int row = 0; row < 6; row++)
	{
		std::string board_row = "|";
		for (int col = 0; col < 7; col++)
		{
			board_row += " ";
			if (game.getBoard(row, col) == 1)
				board_row += "R";
			else if (game.getBoard(row, col) == 2)
				board_row += "Y";
			else
				board_row += ".";
		}
		board_row += " |";
		sendToChannel(channel, board_row);
	}
	
	sendToChannel(channel, "+---------------+");
}

int Bot::botMove(const Connect4 &game)
{
	std::vector<int> validColumns;
	for (int col = 0; col < 7; col++)
	{
		if (game.getBoard(0, col) == 0)
			validColumns.push_back(col);
	}
	
	if (!validColumns.empty())
		return validColumns[rand() % validColumns.size()];
	
	return 0;
}

std::string Bot::generateGameId(const std::string &channel, int player1_fd, int player2_fd)
{
	std::ostringstream oss;
	oss << channel << "_" << player1_fd << "_" << player2_fd << "_" << time(NULL);
	return oss.str();
}

void Bot::sendToChannel(const std::string &channel, const std::string &message)
{
	if (_server)
		_server->sendToChannel(channel, message);
}

std::string Bot::getClientNick(int client_fd)
{
	if (_server)
		return _server->getClientNick(client_fd);
	return "Player" + intToString(client_fd);
}