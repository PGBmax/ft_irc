/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 13:50:38 by nolecler          #+#    #+#             */
/*   Updated: 2025/10/23 20:51:17 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Irc.hpp"

int main(int ac, char **av)
{
	if (ac != 3) {std::cerr << RGB(255,0,0) << "Error:\nCorrect Usage -> ./ircserv <port> <password>" << std::endl; return 0;}
	
	std::string password = av[2];
	size_t		port = static_cast<size_t>(strtod(av[1], NULL));
	
	
	Server serv(port, password);
	std::cout << "---- SERVER ----" << std::endl;
	try
	{
		signal(SIGINT, Server::handleSignal); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, Server::handleSignal); //-> catch the signal (ctrl + \)
		serv.run();
	}
	catch(const std::exception& error)
	{
		serv.closeAllFds(); //-> close the file descriptors
		std::cerr << error.what() << std::endl;
	}
	std::cout << "The Server is Closed!" << std::endl;
}