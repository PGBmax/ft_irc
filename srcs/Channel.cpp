/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/25 17:55:23 by rraumain          #+#    #+#             */
/*   Updated: 2025/11/03 10:44:11 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>

Channel::Channel(const std::string &name) : _name(name), _inviteOnly(false), _topicOperatorOnly(false), _key(""), _userLimit(-1)
{
	std::cout << "Chennel " << name << " constructed" << std::endl;
}

bool Channel::isMember(int fd) const
{
	return _members.count(fd) > 0;
}

bool Channel::isOperator(int fd) const
{
	if (_operators.size() == 1)
		return true;
	return _operators.count(fd) > 0;
}

