/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/25 17:55:23 by rraumain          #+#    #+#             */
/*   Updated: 2025/11/10 03:29:56 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>
#include <algorithm>

Channel::Channel(const std::string &name) : _name(name), _inviteOnly(false), _topicOperatorOnly(false), _key(""), _userLimit(-1)
{
	std::cout << "Channel " << name << " constructed" << std::endl;
}

bool Channel::isMember(int fd) const
{
	if (std::find(_members.begin(), _members.end(), fd) != _members.end())
		return true;
	return false;
}

bool Channel::isOperator(int fd) const
{
	if (_members.size() == 1 && _members[0] == fd)
		return true;
	if (std::find(_members.begin(), _members.end(), fd) == _members.end())
		return false;
	return _operators.count(fd) > 0;
}

void Channel::addOperator(int fd)
{
	 if (isMember(fd))
        _operators.insert(fd);
}

void Channel::removeOperator(int fd)
{
	 _operators.erase(fd);
}
