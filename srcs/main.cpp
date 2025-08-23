/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 13:50:38 by nolecler          #+#    #+#             */
/*   Updated: 2025/08/18 13:50:46 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Server.hpp"
#include "Client.hpp"

int main()
{
	Server ser;
	std::cout << "---- SERVER ----" << std::endl;
	try
	{
		signal(SIGINT, Server::handleSignal); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, Server::handleSignal); //-> catch the signal (ctrl + \)
		ser.initServ(); //-> initialize the server
	}
	catch(const std::exception& error)
	{
		ser.closeAllFds(); //-> close the file descriptors
		std::cerr << error.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
}