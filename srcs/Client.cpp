// bannniere

#include "Client.hpp"
#include "Server.hpp"

Client::Client() : _fd(-1), _ip("")
{
    std::cout << "Client default constructor is created" << std::endl;
}

int Client::getFd() const
{
    return (this->_fd);
}

void Client::setFd(int fd)
{
    this->_fd = fd;
}

void Client::setIp(const std::string& ip)
{
    this->_ip = ip; 
}

std::string Client::getIp() const
{
    return (this->_ip);
}