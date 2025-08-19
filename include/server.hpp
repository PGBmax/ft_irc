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

#ifndef SERVER_HPP
# define SERVER_HPP

// fontions autorisé utile pour le serveur: 
// socket,/ listen,/ accept,/ close,/ setsockopt,/ 
// bind,/ recv,/ send,/ poll/ 
// signal,/  sigaction, fcntl,/ inet_ntoa,/ 

// le cycle de vie complet d’un vrai serveur :
// prépare ce qu’il faut (socket, configuration)
// exécute la boucle (écoute, accepte, lit, écrit)
// gère les clients (ajoute, supprime, communique)
// nettoie à la fin

#include <iostream>
#include <vector> //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> //-> for poll()
#include <csignal> //-> for signal()

class Client;


class server
{
    public :






    private :
        int port;
        int serverFd;
        std::vector<Client> clients;
    
};


#endif
