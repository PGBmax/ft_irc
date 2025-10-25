/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rraumain <rraumain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 18:20:57 by pboucher          #+#    #+#             */
/*   Updated: 2025/10/25 16:44:31 by rraumain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class Client
{
	public :
		int			_fd;
		std::string _in;
		std::string _out;
		bool		_isPasswordValid;
		bool		_registered;
		std::string	_nick;
		std::string	_user;
		std::string	_name;

		Client(int fd);
};