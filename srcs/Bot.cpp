/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 22:41:28 by pboucher          #+#    #+#             */
/*   Updated: 2025/11/17 02:36:58 by pboucher         ###   ########.fr       */
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
	else if (command == "ACCEPT")
		return acceptConnect4(client, channel);
	else if (command == "DECLINE")
		return declineConnect4(client, channel);
	else if (command == "PLAY")
		return playConnect4(message, client, channel);
	else if (command == "FORFEIT")
		return forfeitConnect4(client, channel);
	
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
			it->second.getState() == IN_GAME)
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
				game.incrementMoveCount();
				sendToChannel(channel, "[YELLOW] Bot plays column " + intToString(botColumn + 1) + " (Move " + intToString(game.getMoveCount()) + ")");
				displayBoard(game, channel);
				
				if (checkWin(game, 2))
				{
					sendToChannel(channel, "[WIN] Bot wins! Game over!");
					endGame(gameId, "Bot victory");
					return true;
				} 
				else if (isBoardFull(game))
				{
					sendToChannel(channel, "[TIE] It's a tie! Game over!");
					endGame(gameId, "Tie");
					return true;
				}
				
				game.setCurrentPlayer(1);
				sendToChannel(channel, "[RED] " + getClientNick(client._fd) + "'s turn! Use 'PLAY <column>' (1-7)");
			}
		}
		
		return true;
	}
	
	if (_server)
	{
		sendToChannel(channel, "[CHALLENGE] " + getClientNick(client._fd) + " challenges " + targetPlayer + " to Connect 4!");
		sendToChannel(channel, "[INFO] " + targetPlayer + ", type 'ACCEPT' or 'DECLINE' !");
		
		std::string gameId = generateGameId(channel, client._fd, -2) + "_vs_" + targetPlayer;
		Connect4 &game = _games[gameId];
		game.setPlayer1Fd(client._fd);
		game.setPlayer2Fd(-2);
		game.setVsBot(false);
		game.setChannel(channel);
		game.setState(WAITING_FOR_ACCEPT);
		
		return true;
	}
	
	sendToChannel(channel, "[ERROR] Error starting game!");
	return true;
}

bool Bot::acceptConnect4(Client &client, const std::string &channel)
{
	std::string clientNick = getClientNick(client._fd);
	std::string acceptedGameId = "";
	int challengerFd = -1;
	
	for (std::map<std::string, Connect4>::iterator it = _games.begin(); it != _games.end(); ++it)
	{
		if (it->second.getChannel() == channel && 
			it->second.getState() == WAITING_FOR_ACCEPT &&
			it->second.getPlayer2Fd() == -2 &&
			it->first.find("_vs_" + clientNick) != std::string::npos)
		{
			acceptedGameId = it->first;
			challengerFd = it->second.getPlayer1Fd();
			break;
		}
	}
	
	if (acceptedGameId.empty())
	{
		sendToChannel(channel, "[ERROR] No pending Connect 4 challenge found for you!");
		return true;
	}
	
	std::vector<std::string> gamesToRemove;
	for (std::map<std::string, Connect4>::iterator it = _games.begin(); it != _games.end(); ++it)
	{
		if (it->second.getState() == WAITING_FOR_ACCEPT &&
			it->second.getPlayer1Fd() == challengerFd &&
			it->first != acceptedGameId)
		{
			gamesToRemove.push_back(it->first);
		}
	}
	
	for (std::vector<std::string>::iterator it = gamesToRemove.begin(); it != gamesToRemove.end(); ++it)
	{
		_games.erase(*it);
	}
	
	Connect4 &game = _games[acceptedGameId];
	game.setPlayer2Fd(client._fd);
	game.setState(IN_GAME);
	
	srand(time(NULL));
	game.setCurrentPlayer((rand() % 2) + 1);
	
	sendToChannel(channel, "[GAME] Game accepted! " + getClientNick(game.getPlayer1Fd()) + " [R] vs " + getClientNick(game.getPlayer2Fd()) + " [Y]");
	displayBoard(game, channel);
	
	if (game.getCurrentPlayer() == 1)
		sendToChannel(channel, "[RED] " + getClientNick(game.getPlayer1Fd()) + "'s turn! Use 'PLAY <column>' (1-7)");
	else
		sendToChannel(channel, "[YELLOW] " + getClientNick(game.getPlayer2Fd()) + "'s turn! Use 'PLAY <column>' (1-7)");
	
	return true;
}

bool Bot::playConnect4(t_message &message, Client &client, const std::string &channel)
{
	std::istringstream iss(message.params[1]);
	std::string command;
	int column = 0;
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
	
	activeGame->incrementMoveCount();
	
	std::string playerSymbol = (activeGame->getCurrentPlayer() == 1) ? "[RED]" : "[YELLOW]";
	sendToChannel(channel, playerSymbol + " " + getClientNick(client._fd) + " plays column " + intToString(column + 1) + " (Move " + intToString(activeGame->getMoveCount()) + ")");
	
	displayBoard(*activeGame, channel);
	
	if (checkWin(*activeGame, activeGame->getCurrentPlayer()))
	{
		sendToChannel(channel, "[WIN] " + getClientNick(client._fd) + " wins! Game over!");
		endGame(activeGameId, getClientNick(client._fd) + " victory");
		return true;
	}
	
	if (isBoardFull(*activeGame))
	{
		sendToChannel(channel, "[TIE] It's a tie! Game over!");
		endGame(activeGameId, "Tie");
		return true;
	}
	
	activeGame->setCurrentPlayer((activeGame->getCurrentPlayer() == 1) ? 2 : 1);
	
	if (activeGame->isVsBot() && activeGame->getCurrentPlayer() == 2)
	{
		sendToChannel(channel, "[YELLOW] Bot's turn!");
		
		int botColumn = botMove(*activeGame);
		if (dropPiece(*activeGame, botColumn))
		{
			activeGame->incrementMoveCount();
			sendToChannel(channel, "[YELLOW] Bot plays column " + intToString(botColumn + 1) + " (Move " + intToString(activeGame->getMoveCount()) + ")");
			displayBoard(*activeGame, channel);
			
			if (checkWin(*activeGame, 2))
			{
				sendToChannel(channel, "[WIN] Bot wins! Game over!");
				endGame(activeGameId, "Bot victory");
				return true;
			}
			else if (isBoardFull(*activeGame))
			{
				sendToChannel(channel, "[TIE] It's a tie! Game over!");
				endGame(activeGameId, "Tie");
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

bool Bot::forfeitConnect4(Client &client, const std::string &channel)
{
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
		sendToChannel(channel, "[ERROR] You're not in an active game to forfeit!");
		return true;
	}
	
	std::string forfeitingPlayer = getClientNick(client._fd);
	std::string winningPlayer;
	
	if (activeGame->isVsBot())
		winningPlayer = "Bot";
	else 
	{
		int winnerFd = (activeGame->getPlayer1Fd() == client._fd) ? activeGame->getPlayer2Fd() : activeGame->getPlayer1Fd();
		winningPlayer = getClientNick(winnerFd);
	}
	
	sendToChannel(channel, "[FORFEIT] " + forfeitingPlayer + " forfeits the game!");
	sendToChannel(channel, "[WIN] " + winningPlayer + " wins by forfeit!");
	
	endGame(activeGameId, forfeitingPlayer + " forfeited");
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

bool Bot::isBoardFull(const Connect4 &game)
{
	for (int col = 0; col < 7; col++)
	{
		if (game.getBoard(0, col) == 0)
			return false;
	}
	return true;
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
	for (int col = 0; col < 7; col++)
	{
		if (game.getBoard(0, col) == 0)
		{
			Connect4 tempGame = game;
			for (int row = 5; row >= 0; row--)
			{
				if (tempGame.getBoard(row, col) == 0)
				{
					tempGame.setBoard(row, col, 2);
					break;
				}
			}
			if (checkWin(tempGame, 2))
				return col;
		}
	}
	
	for (int col = 0; col < 7; col++)
	{
		if (game.getBoard(0, col) == 0)
		{
			Connect4 tempGame = game;
			for (int row = 5; row >= 0; row--)
			{
				if (tempGame.getBoard(row, col) == 0)
				{
					tempGame.setBoard(row, col, 1);
					break;
				}
			}
			if (checkWin(tempGame, 1))
				return col;
		}
	}
	
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
	static int gameCounter = 0;
	std::ostringstream oss;
	oss << channel << "_" << player1_fd << "_" << player2_fd << "_" << (++gameCounter);
	return oss.str();
}

void Bot::endGame(const std::string &gameId, const std::string &reason)
{
	std::map<std::string, Connect4>::iterator it = _games.find(gameId);
	if (it != _games.end())
	{
		sendToChannel(it->second.getChannel(), "[END] Game ended: " + reason);
		_games.erase(it);
	}
}

void Bot::sendToChannel(const std::string &channel, const std::string &message)
{
	if (_server)
		_server->botSendToChannel(channel, message);
}

void Bot::sendToClient(int client_fd, const std::string &message)
{
	if (_server)
		_server->botSendToClient(client_fd, message);
}

std::string Bot::getClientNick(int client_fd)
{
	if (_server)
		return _server->botGetClientNick(client_fd);
	return "Player" + intToString(client_fd);
}

bool Bot::isClientInChannel(int client_fd, const std::string &channel)
{
	if (_server)
		return _server->botIsClientInChannel(client_fd, channel);
	return false;
}

void Bot::onClientDisconnected(int client_fd)
{
	std::vector<std::string> gamesToEnd;
	
	for (std::map<std::string, Connect4>::iterator it = _games.begin(); it != _games.end(); ++it)
	{
		const Connect4 &game = it->second;
		if (game.getPlayer1Fd() == client_fd || game.getPlayer2Fd() == client_fd)
			gamesToEnd.push_back(it->first);
	}
	
	for (std::vector<std::string>::iterator it = gamesToEnd.begin(); it != gamesToEnd.end(); ++it)
		endGame(*it, "Player disconnected");
}

void Bot::onPlayerLeftChannel(int client_fd, const std::string &channel)
{
	std::vector<std::string> gamesToEnd;
	
	for (std::map<std::string, Connect4>::iterator it = _games.begin(); it != _games.end(); ++it)
	{
		const Connect4 &game = it->second;
		if (game.getChannel() == channel && (game.getPlayer1Fd() == client_fd || game.getPlayer2Fd() == client_fd))
			gamesToEnd.push_back(it->first);
	}
	
	for (std::vector<std::string>::iterator it = gamesToEnd.begin(); it != gamesToEnd.end(); ++it)
		endGame(*it, "Player left channel");
}

bool Bot::declineConnect4(Client &client, const std::string &channel)
{
	std::string clientNick = getClientNick(client._fd);
	std::string declinedGameId = "";
	int challengerFd = -1;
	
	for (std::map<std::string, Connect4>::iterator it = _games.begin(); it != _games.end(); ++it)
	{
		if (it->second.getChannel() == channel && 
			it->second.getState() == WAITING_FOR_ACCEPT &&
			it->second.getPlayer2Fd() == -2 &&
			it->first.find("_vs_" + clientNick) != std::string::npos)
		{
			declinedGameId = it->first;
			challengerFd = it->second.getPlayer1Fd();
			break;
		}
	}
	
	if (declinedGameId.empty())
	{
		sendToChannel(channel, "[ERROR] No pending Connect 4 challenge found for you!");
		return true;
	}
	
	sendToChannel(channel, "[DECLINE] " + clientNick + " declined the Connect 4 challenge from " + getClientNick(challengerFd) + "!");
	_games.erase(declinedGameId);
	
	return true;
}

