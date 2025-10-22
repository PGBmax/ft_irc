/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 18:20:57 by pboucher          #+#    #+#             */
/*   Updated: 2025/10/22 19:18:56 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

class Client
{
    public :
        Client();

        int getFd() const;
        void setFd(int fd);

        void setIp(const std::string& ip);
        std::string getIp() const; // a voir si utile ou pas

    private :
        int _fd;
        std::string _ip;

};