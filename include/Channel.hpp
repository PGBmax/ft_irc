/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rraumain <rraumain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/25 17:30:35 by rraumain          #+#    #+#             */
/*   Updated: 2025/10/25 18:34:28 by rraumain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <set>

class Channel
{
	public:
		std::string		_name;
		std::string		_topic;
		std::set<int>	_members;
		std::set<int>	_operators;
		std::set<int>	_invited;
		bool			_inviteOnly;
		bool			_topicOperatorOnly;
		std::string		_key;
		int				_userLimit;

		Channel(const std::string &name);
		
		bool isMember(int fd) const;
		bool isOperator(int fd) const;
};