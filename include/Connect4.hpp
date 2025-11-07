/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connect4.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 22:41:22 by pboucher          #+#    #+#             */
/*   Updated: 2025/11/06 22:41:23 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

enum GameState {
	WAITING_FOR_OPPONENT,
	IN_GAME,
	GAME_OVER
};

class Connect4
{
	public:
		Connect4();
		~Connect4();
		
		int		getBoard(int row, int col) const;
		void	setBoard(int row, int col, int value);
		
		int		getPlayer1Fd() const;
		int		getPlayer2Fd() const;
		int		getCurrentPlayer() const;
		void	setPlayer1Fd(int fd);
		void	setPlayer2Fd(int fd);
		void	setCurrentPlayer(int player);
		
		const 	std::string& getChannel() const;
		void	setChannel(const std::string& channel);
		
		GameState 	getState() const;
		void		setState(GameState state);

		bool 	isVsBot() const;
		void	setVsBot(bool vs_bot);

	private:
		int _board[6][7];
		int _player1_fd;
		int _player2_fd;
		int _current_player;
		std::string _channel;
		GameState _state;
		bool _vs_bot;
	
};