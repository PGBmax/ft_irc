/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 19:17:32 by pboucher          #+#    #+#             */
/*   Updated: 2025/10/22 19:17:34 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Irc.hpp"

Client::Client() : _fd(-1), _ip("")
{
    std::cout << "Client default constructor is created" << std::endl;
}

int Client::getFd() const
{
    return (this->_fd);
}

void Client::setFd(int fd)
{
    this->_fd = fd;
}

void Client::setIp(const std::string& ip)
{
    this->_ip = ip; 
}

std::string Client::getIp() const
{
    return (this->_ip);
}