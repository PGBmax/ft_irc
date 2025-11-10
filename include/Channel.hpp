/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/25 17:30:35 by rraumain          #+#    #+#             */
/*   Updated: 2025/11/08 18:33:06 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <set>
#include <vector>

class Channel
{
	public:
		std::string		_name;
		std::string		_topic;
		std::vector<int> _members;
		std::set<int>	_operators;
		std::set<int>	_invited;
		bool			_inviteOnly;
		bool			_topicOperatorOnly;
		std::string		_key;
		int				_userLimit;

		Channel(const std::string &name);
		
		bool isMember(int fd) const;
		bool isOperator(int fd) const;

		void addOperator(int fd);
		void removeOperator(int fd);
};