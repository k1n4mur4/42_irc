#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <csignal>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.hpp"

#define RED "\033[1;31m"
#define WHI "\033[0;37m"
#define GRE "\033[1;32m"
#define YEL "\033[1;33m"

class Server
{
	public:
		Server(){serSocketFd_ = -1;}
		Server(int port, std::string password){
			port_ = port;
			password_ = password;

			std::cout << "Server Config" << std::endl;
			std::cout << "PORT:\t\t" << port_ << std::endl;
			std::cout << "PASSWORD:\t" << password_ << std::endl;
		}

		void		ServerInit();
		void		SerSocket();
		void		AcceptNewClient();
		void		ReceiveNewData(int fd);

		static void	signalHandler(int signum);
	
		void		setPort(int port){port_ = port;};
		int			getPort(){return port_;};
		void		setPassword(std::string password){password_ = password;};
		std::string	getPassword(){return password_;};
		void		setSignal(bool signal){signal_ = signal;};
		bool		getSignal(){return signal_;};
		void		closeFds();
		void		clearClients(int fd);

	private:
		int							port_;
		std::string					password_;
		int							serSocketFd_;
		static bool					signal_;
		std::vector<Client>			clients_;
		std::vector<struct pollfd>	fds_;
};

#endif
