/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/25 17:55:23 by rraumain          #+#    #+#             */
/*   Updated: 2025/11/06 09:14:38 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>

Channel::Channel(const std::string &name) : _name(name), _inviteOnly(false), _topicOperatorOnly(false), _key(""), _userLimit(-1)
{
	std::cout << "Channel " << name << " constructed" << std::endl;
}

bool Channel::isMember(int fd) const
{
	return _members.count(fd) > 0;
}

bool Channel::isOperator(int fd) const
{
	//s'il n;y a qu'un seul membre et que c'est bien lui
	if (_members.size() == 1 && _members.count(fd))
		return true;
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
