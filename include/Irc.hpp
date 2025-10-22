/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 18:20:51 by pboucher          #+#    #+#             */
/*   Updated: 2025/10/22 19:21:59 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <csignal> //-> for signal()
#include <poll.h> // -> for poll()
#include <cstdlib>
#include <cstring>

# define CLR "\033[0m"
# define BOLD "\033[1m"
# define ITAL "\033[3m"
# define UNDL "\033[4m"
# define DUNDL "\033[21m"
# define STRK "\033[9m"
# define BLNK "\033[5m"

# define BLK "\033[0;30m"
# define RED "\033[0;31m"
# define GRN "\033[0;32m"
# define YLW "\033[0;33m"
# define BLU "\033[0;34m"
# define PRP "\033[0;35m"
# define CYN "\033[0;36m"
# define WHT "\033[0;37m"

# define RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"