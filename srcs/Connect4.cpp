/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connect4.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 22:50:32 by pboucher          #+#    #+#             */
/*   Updated: 2025/11/10 03:44:13 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connect4.hpp"

Connect4::Connect4() : _player1_fd(-1), _player2_fd(-1), _current_player(1), _state(WAITING_FOR_OPPONENT), _vs_bot(false), _move_count(0)
{
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
			_board[i][j] = 0;
	}
}

Connect4::~Connect4() {}

int Connect4::getBoard(int row, int col) const {
	return _board[row][col];
}

void Connect4::setBoard(int row, int col, int value) {
	_board[row][col] = value;
}

int Connect4::getPlayer1Fd() const {
	return _player1_fd;
}

void Connect4::setPlayer1Fd(int fd) {
	_player1_fd = fd;
}

int Connect4::getPlayer2Fd() const {
	return _player2_fd;
}

void Connect4::setPlayer2Fd(int fd) {
	_player2_fd = fd;
}

int Connect4::getCurrentPlayer() const {
	return _current_player;
}

void Connect4::setCurrentPlayer(int player) {
	_current_player = player;
}

const std::string& Connect4::getChannel() const {
	return _channel;
}

void Connect4::setChannel(const std::string& channel) {
	_channel = channel;
}

GameState Connect4::getState() const {
	return _state;
}

void Connect4::setState(GameState state) {
	_state = state;
}

bool Connect4::isVsBot() const {
	return _vs_bot;
}

void Connect4::setVsBot(bool vs_bot) {
	_vs_bot = vs_bot;
}

int Connect4::getMoveCount() const {
	return _move_count;
}

void Connect4::setMoveCount(int count) {
	_move_count = count;
}

void Connect4::incrementMoveCount() {
	_move_count++;
}