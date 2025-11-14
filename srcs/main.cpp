/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 13:50:38 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/14 10:18:19 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Decorator.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>

void signal_handler(int pid)
{
    if (pid == SIGINT || pid == SIGQUIT)
        throw Server::SignalHandler();
}

int main(int ac, char **av)
{
    if (ac != 3) {
        std::cerr << RGB(255,0,0) << "Usage: " << av[0] << " <port> <password>" << std::endl;
        return 1;
    }
    int port = std::atoi(av[1]);
    std::string password = av[2];

    try {
        Server server(port, password);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGINT, signal_handler);
        signal(SIGQUIT, signal_handler);
        server.run();
    } catch (const std::exception &error) {
        std::cerr << RGB(255,0,0) << "Error: " << error.what() << CLR << std::endl;
        return 2;
    }
    return 0;
}

// join plusierus channel mauvais mot de passe key1, a gerer
// accepte join #test1,#test2,#test3 key1,key2  accepte join #test1 key1,,,,,, autant de virgule est accepter
