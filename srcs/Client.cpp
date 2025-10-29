/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rraumain <rraumain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 19:17:32 by pboucher          #+#    #+#             */
/*   Updated: 2025/10/25 16:44:41 by rraumain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <iostream>

Client::Client(int fd) : _fd(fd), _isPasswordValid(false), _registered(false)
{
    std::cout << "Client " << fd << " constructed" << std::endl;
}

// int Client::getFd() const
// {
//     return (this->_fd);
// }

// void Client::setFd(int fd)
// {
//     this->_fd = fd;
// }

// void Client::setIp(const std::string& ip)
// {
//     this->_ip = ip; 
// }

// std::string Client::getIp() const
// {
//     return (this->_ip);
// }