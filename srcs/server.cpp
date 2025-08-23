/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:57:28 by nolecler          #+#    #+#             */
/*   Updated: 2025/08/19 10:57:47 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"
#include "Client.hpp"

#include <iostream>
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <csignal> //-> for signal()


Server::Server() : _serverSocketFd(-1), _port(0)
{
    std::cout << "Server constructor is created" << std::endl;
}

bool Server::_signal = false;

void Server::createSocket()
{

    int opt = 1;
    struct pollfd NewPoll;
    struct sockaddr_in serverAddress;// structure d'adresse
    serverAddress.sin_family  = AF_INET; // Ce socket utilisera le protocole Internet IPv4.
    serverAddress.sin_addr.s_addr = INADDR_ANY; // “Le serveur acceptera les connexions sur n’importe quelle interface réseau de la machine.”
    serverAddress.sin_port = htons(this->_port); // le serveur est associé à un port précis pour recevoir les connexions.


    // crée un socket serveur TCP en IPv4.
    this->_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_serverSocketFd == - 1)
        throw(std::runtime_error("creation socket server failed"));

   
    // permettre de réutiliser le port rapidement
    if(setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		  throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));


    // met les sockets en non bloquant
    if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1)
		  throw(std::runtime_error("failed to set option (O_NONBLOCK) on socket"));


    // assigner l’adresse IP + port
    if (bind(_serverSocketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
		  throw(std::runtime_error("failed to bind socket"));

    // mettre le socket en mode écoute
    if (listen(_serverSocketFd, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() failed"));

    // ajout du socket serveur dans la liste a surveiller par poll()
    NewPoll.fd = _serverSocketFd; //-> ajout du socket du serveur au pollfd pour que poll() surveille les evnmt de ce socket
	NewPoll.events = POLLIN; //detecte quand un client tente de se connecter
	NewPoll.revents = 0; //evnmt reellement survenu 0 au depart
	_fds.push_back(NewPoll); //on ajoute le socket serveur à la liste des sockets surveillés par poll().

}

void Server::initServ()
{
    this->_port = 4444; // porte d'entree pour les clients

    createSocket();
    std::cout << "Server socket <" << _serverSocketFd << "> connected." << std::endl;
    std::cout << "Waiting for incoming client connections..." << std::endl;

    while (Server::_signal == false) // _signal est static 
    {
        if ((poll(&_fds[0], _fds.size(), -1) == -1) && Server::_signal == false)
            throw(std::runtime_error("poll() failed"));

        for(size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents & POLLIN)
            {
                if (_fds[i].fd == _serverSocketFd)
                    acceptNewClient();
                else
                    handleClientData(_fds[i].fd);
            }
            // if (_fds[i].revents & POLLHUP || _fds[i].revents & POLLERR)
            // {
            //     std::cout << "Client <" << _fds[i].fd << "> Disconnected or Error" << std::endl;
            //     close(_fds[i].fd);
            //     ClearClients(_fds[i].fd); // retire le client et le pollfd
            //     i--; // ajuster l’indice après suppression
            // }
        }
    }
    closeAllFds();
}


void Server::acceptNewClient()
{

    Client cli;
    struct sockaddr_in newClientAddress;
    struct pollfd newClientPoll;
    socklen_t len = sizeof(newClientAddress);//taille de la structure d’adresse client

    //on accepte une nouvelle connexion sur le socket serveur
    //et le descripteur du client connecté est retourné
    int clientFd = accept(_serverSocketFd, (sockaddr *)&(newClientAddress), &len)
    if (clientFd == -1)
    {
        std::cout << "accept() failed" << std::endl;
        return ;
    }
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) //on change le flag du fd en non bloquant
    {
        std::cout << "fcntl() failed" << std::endl;
        return;
    }
    newClientPoll.fd = clientFd; // ajout du client dans le tableau de fd poll
    newClientPoll.events = POLLIN; //on active le flag pollin pr surveiller si le client a envoyer des donnees a lire
    newClientPoll.revents = 0;

    cli.setFd(clientFd);//on donne a _fd le fd que accept() vient de renvoyer pour ce client.
    cli.setIp(inet_ntoa((newClientAddress.sin_addr)));//convertit l’adress IP du client en string et la stocke dans l'objet Client.
	_clients.push_back(cli);// ajout du client a la fin du vecteur de _clients 

    std::cout << "Client <" << clientFd << "> Connected" << std::endl;
}


void Server::handleClientData(int fd)
{
    Identifier le client via le fd.
    Lire les données envoyées par le client.
    Vérifier si le client a fermé la connexion ou si erreur.
    Traiter les données reçues.
    Envoyer une réponse si nécessaire.
}

