
#pragma once
#include <string>



class Client
{
    public :
        Client();

        int getFd() const;
        void setFd(int fd);

        void setIp(const std::string& ip);
        std::string getIp() const; // a voir si utile ou pas

    private :
        int _fd;
        std::string _ip;

};