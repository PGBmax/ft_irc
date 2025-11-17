/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nolecler <nolecler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 16:38:24 by nolecler          #+#    #+#             */
/*   Updated: 2025/11/17 16:22:30 by nolecler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <algorithm>

bool Server::join(t_message &message, Client &client)
{
    if (message.params.empty() || message.params.size() > 2)
    {
        sendClient(461, client, "Not enough or too many parameters");
        return false;
    }
	
    std::vector<std::string> channels; 
    size_t start = 0;
    std::string names = message.params[0];
    size_t pos;

    while ((pos = names.find(',', start)) != std::string::npos)
    {
        channels.push_back(names.substr(start, pos - start));
        start = pos + 1;
    }
    channels.push_back(names.substr(start));

    std::vector<std::string> keys;
    if (message.params.size() > 1)
    {
        start = 0;
        std::string keyStr = message.params[1];
        while ((pos = keyStr.find(',', start)) != std::string::npos)
        {
            keys.push_back(keyStr.substr(start, pos - start));
            start = pos + 1;
        }
        keys.push_back(keyStr.substr(start));
    }

    while (keys.size() < channels.size())
        keys.push_back("");

    bool joined = false;

    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string name = channels[i];
        std::string key = keys[i];

        if (name.empty() || name[0] != '#')
        {
            sendClient(403, client, "Invalid channel name: " + name);
            continue;
        }
		if (_channels.find(name) == _channels.end())
		{
    		Channel newChannel(name);
    		if (!key.empty())
        		newChannel._key = key;
    		_channels.insert(std::map<std::string, Channel>::value_type(name, newChannel));
		}

        Channel &channel = getChannel(name);

        if (channel._inviteOnly && channel._invited.count(client._fd) == 0)
        {
            sendClient(473, client, name + " Cannot join channel (+i)");
            continue;
        }

        // Vérifier que la clé ne contient pas de virgule
        // if (key.find(',') != std::string::npos)
        // {
        //     sendClient(475, client, name + " Cannot join channel (+k)");
        //     continue;
        // }

        
        if (!channel._key.empty() && key != channel._key)
        {
            sendClient(475, client, name + " Cannot join channel (+k)");
            continue;
        }

        if (channel._userLimit > 0 && static_cast<int>(channel._members.size()) >= channel._userLimit)
        {
            sendClient(471, client, name + " Cannot join channel (+l)");
            continue;
        }
		
		std::vector<int>::iterator it = std::find(channel._members.begin(), channel._members.end(), client._fd);
		if (it != channel._members.end())
		{
    		sendClient(443, client, name + " is already on channel");
    		continue; 
		}
		else
			channel._members.push_back(client._fd);
        
        if (channel._members.size() == 1) 
            channel._operators.insert(client._fd);

        
        std::string line = ":" + client._nick + " JOIN " + name;
        sendInChannel(channel, -1, line); 

		
        if (channel._topic.empty())
            sendClient(331, client, name + " No topic is set");
        else
            sendClient(332, client, name + " " + channel._topic);

        std::string nickList; // Afficher liste des membres
        for (size_t j = 0; j < channel._members.size(); ++j)
        {
            Client &c = getClient(channel._members[j]);
            nickList += c._nick + " ";
        }
        sendClient(353, client, name + " NAMES LIST: " + nickList);
        sendClient(366, client, name + " End of NAMES list");

        joined = true;
    }
    return joined;
}



// bool Server::join(t_message &message, Client &client)
// {
//     if (message.params.empty() || message.params.size() > 2)
//     {
//         sendClient(461, client, "Not enough or too many parameters");
//         return false;
//     }
    
//     std::vector<std::string> channels; //  Découper #chan1,#chan2,#chan3
//     size_t start = 0;
//     std::string names = message.params[0]; // #chan1,#chan2,#chan3
//     size_t pos; // position du virgule

//     while ((pos = names.find(',', start))  != std::string::npos)
//     {
//         channels.push_back(names.substr(start, pos - start)); //extrait #chan1
//         start = pos + 1;
//     }
//     channels.push_back(names.substr(start)); // extrait #chan3

//     std::vector<std::string> keys; // Découper les clés s'il y a
//     if (message.params.size() > 1) // si cle fournit
//     {
//         std::string keyStr = message.params[1]; //key1,key2,key3
//         start = 0;
//         pos = 0;
//         while ((pos = keyStr.find(',', start)) != std::string::npos)
//         {
//             keys.push_back(keyStr.substr(start, pos - start)); // pos = 4;
//             start = pos + 1;
//         }
//         keys.push_back(keyStr.substr(start));
//     }
    
    
//     while (keys.size() < channels.size()) // Si moins de clés que de channels, remplir avec "" // FAUX a modifier
//         keys.push_back("");
    
//     bool joined = false;

//     for (size_t i = 0; i < channels.size(); ++i)
//     {
//         std::string name = channels[i];
//         std::string key = keys[i];

//         if (name.empty() || name[0] != '#')
//         {
//             sendClient(403, client, "Invalid channel name: " + name);
//             continue;
//         }
// 		if (_channels.find(name) == _channels.end())
// 		{
//     		Channel newChannel(name);
//     		if (!key.empty()) // si le client a fourni une clé, la définir pour ce channel
//         		newChannel._key = key;
            
//     		_channels.insert(std::map<std::string, Channel>::value_type(name, newChannel));
// 		}

//         Channel &channel = getChannel(name);
		
//         if (channel._inviteOnly && !channel._invited.count(client._fd))
//         {
//             sendClient(473, client, name + " Cannot join channel (+i)");
//             continue;
//         }
		
//         // si la clé contient une virgule → invalide
//         if (key.find(',') != std::string::npos)
//         {
//             sendClient(475, client, name + " Cannot join channel (+k)");
//             continue;
//         }
        
//   		if (!channel._key.empty() && key != channel._key)
//         {
//             sendClient(475, client, name + " Cannot join channel (+k)");
//             continue;
//         }
		
//         if (channel._userLimit > 0 && static_cast<int>(channel._members.size()) >= channel._userLimit)
//         {
//             sendClient(471, client, name + " Cannot join channel (+l)");
//             continue;
//         }
		
// 		std::vector<int>::iterator it = std::find(channel._members.begin(), channel._members.end(), client._fd);
// 		if (it != channel._members.end())
// 		{
//     		sendClient(443, client, name + " is already on channel");
//     		continue; // passe au channel suivant
// 		}
// 		else
// 			channel._members.push_back(client._fd); // Ajouter le client si pas déjà membre
        
//         if (channel._members.size() == 1) // Premier membre devient opérateur
//             channel._operators.insert(client._fd);
    
//         std::string line = ":" + client._nick + " JOIN " + name;
//         sendInChannel(channel, -1, line); // Informer tout le monde

// 		//Afficher le topic + liste de membre au nouveau client
//         if (channel._topic.empty())
//             sendClient(331, client, name + " No topic is set");
//         else
//             sendClient(332, client, name + " " + channel._topic);
			
//         std::string nickList;
//         for (size_t j = 0; j < channel._members.size(); ++j)
//         {
//             Client &c = getClient(channel._members[j]);
//             nickList += c._nick + " ";
//         }
//         sendClient(353, client, name + " NAMES LIST: " + nickList);
//         sendClient(366, client, name + " End of NAMES list");

//         joined = true;
//     }
//     return joined;
// }
