/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rraumain <rraumain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:21:21 by nolecler          #+#    #+#             */
/*   Updated: 2025/10/25 18:24:43 by rraumain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Channel.hpp"
#include <string>
#include <poll.h>
#include <vector>
#include <map>

typedef struct s_message
{
    std::string command; 
    std::vector<std::string> params;
} t_message;

class Server
{
    public :
        Server(int port, const std::string &password);
        ~Server();

        void run();

    private :
        int                             _port;
        std::string                     _password;
        int                             _listenFd;
        std::vector<pollfd>             _pfds;
        std::map<int, Client>           _clients;

        void setupListenSocket();
        void acceptNewClient();
        void readFromClient(size_t id);
        void closeClient(size_t id);
        Client &getClient(size_t id);
        void handleLine(size_t id, const std::string &line);

        void ping(t_message &message, Client &client, size_t id);
        void pass(t_message &message, Client &client);
        bool nick(t_message &message, Client &client);
        bool user(t_message &message, Client &client);
        void userRegister(t_message &message, Client &client);
};
