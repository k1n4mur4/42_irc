#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

class Client
{
	private:
		int fd_;
		std::string ip_add_;
	public:
		Client(){};
		int		getFd(){return fd_;}
		void	setFd(int fd){fd_ = fd;}

		void	setIpAdd(std::string ipadd){ip_add_ = ipadd;}
};

#endif
