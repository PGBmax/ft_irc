/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 19:17:32 by pboucher          #+#    #+#             */
/*   Updated: 2025/11/17 02:10:45 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <iostream>

Client::Client(int fd) : _fd(fd), _isPasswordValid(false), _registered(false)
{
    std::cout << "Client " << fd << " constructed" << std::endl;
}


