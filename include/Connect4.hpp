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
#include <ctime>

enum GameState {
	WAITING_FOR_OPPONENT,
	WAITING_FOR_ACCEPT,
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
		void	setPlayer1Fd(int fd);
		int 	getPlayer2Fd() const;
		void 	setPlayer2Fd(int fd);
		int 	getCurrentPlayer() const;
		void 	setCurrentPlayer(int player);

		const std::string&	getChannel() const;
		void				setChannel(const std::string& channel);
		GameState			getState() const;
		void				setState(GameState state);

		bool	isVsBot() const;
		void	setVsBot(bool vs_bot);

		int		getMoveCount() const;
		void	setMoveCount(int count);
		void	incrementMoveCount();

	private:
		int _board[6][7];
		int _player1_fd;
		int _player2_fd;
		int _current_player;
		std::string _channel;
		GameState _state;
		bool _vs_bot;
		int _move_count;
};