/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:21:21 by nolecler          #+#    #+#             */
/*   Updated: 2025/08/19 12:45:34 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// fontions autorisé utile pour le serveur: 
// socket,/ listen,/ accept,/ close,/ setsockopt,/ 
// bind,/ recv,/ send,/ poll/ 
// signal,/  sigaction, fcntl,/ inet_ntoa,/ 



#include <vector> //-> for vector
#include <poll.h> //-> for poll()
#include "Client.hpp"


class Server
{
    public :
        Server();
        void initServ();
        void createSocket(); // run()

        //sert uniquement à accepter une nouvelle connexion entrante sur le socket d’écoute
        void acceptNewClient();// on ecoute si un client est en demande de connnection et on accepte
        void handleClientData(int fd);// gerer tous ce que le client fait

        static void handleSignal(int signal);

        void closeAllFds();
        void clearClient(int fd);

    private :
        int _port;
        int _serverSocketFd;
        std::vector<Client> _clients;
        std::vector<struct pollfd> _fds;
        static bool _signal; // un membre static aapartient a la classe pas a chaque objet 
        //donc il n existe qu'une seul fois pour le serveur
    
};
