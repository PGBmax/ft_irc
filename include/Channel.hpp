/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/25 17:30:35 by rraumain          #+#    #+#             */
/*   Updated: 2025/11/05 16:58:36 by nolecler         ###   ########.fr       */
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
		bool			_inviteOnly; //i
		bool			_topicOperatorOnly; //t
		std::string		_key; //k
		int				_userLimit; //l

		Channel(const std::string &name);
		
		bool isMember(int fd) const;
		bool isOperator(int fd) const;

		void addOperator(int fd);
		void removeOperator(int fd);
};