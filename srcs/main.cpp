/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rraumain <rraumain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 13:50:38 by nolecler          #+#    #+#             */
/*   Updated: 2025/10/24 15:45:07 by rraumain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Decorator.hpp"
#include <iostream>
#include <cstdlib>

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
        server.run();
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 2;
    }
    return 0;
}