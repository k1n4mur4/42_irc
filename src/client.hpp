#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

class Client
{
	public:
		Client(){};
		int		getFd(){return fd_;}
		void	setFd(int fd){fd_ = fd;}

		std::string	getIpAdd() {return ip_add_;}
		void		setIpAdd(std::string ipadd){ip_add_ = ipadd;}

	private:
		int fd_;
		std::string ip_add_;
};

#endif
