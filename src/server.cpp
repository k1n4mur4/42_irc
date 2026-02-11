#include "server.hpp"

void	Server::clearClients(int fd){
	for(size_t i = 0; i < fds_.size(); i++){
		if (fds_[i].fd == fd) {
			fds_.erase(fds_.begin() + i);
			break;
		}
	}
	for(size_t i = 0; i < clients_.size(); i++){
		if (clients_[i].getFd() == fd) {
			clients_.erase(clients_.begin() + i);
			break;
		}
	}
}

void	Server::signalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::signal_ = true;
}

void	Server::closeFds(){
	for(size_t i = 0; i < clients_.size(); i++){
		std::cout << RED << "Client <" << clients_[i].getFd() << "> Disconnected" << WHI << std::endl;
		close(clients_[i].getFd());
	}
	if (serSocketFd_ != -1){
		std::cout << RED << "Server <" << serSocketFd_ << "> Disconnected" << WHI << std::endl;
		close(serSocketFd_);
	}
}

void	Server::SerSocket()
{
	struct sockaddr_in add;
	struct pollfd NewPoll;
	add.sin_family = AF_INET;
	add.sin_port = htons(this->port_);
	add.sin_addr.s_addr = INADDR_ANY;

	serSocketFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if(serSocketFd_ == -1)
		throw(std::runtime_error("faild to create socket"));

	int en = 1;
	if(setsockopt(serSocketFd_, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(serSocketFd_, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(serSocketFd_, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(serSocketFd_, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));

	NewPoll.fd = serSocketFd_;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;
	fds_.push_back(NewPoll);
}

void Server::AcceptNewClient()
{
	Client cli;
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(serSocketFd_, (sockaddr *)&(cliadd), &len);
	if (incofd == -1) {
		std::cout << "accept() failed" << std::endl;
		return;
	}

	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) {
		std::cout << "fcntl() failed" << std::endl;
		return;
	}

	NewPoll.fd = incofd;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;

	cli.setFd(incofd);
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
	clients_.push_back(cli);
	fds_.push_back(NewPoll);

	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::ReceiveNewData(int fd)
{
	char buff[1024];

	memset(buff, 0, sizeof(buff));

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0) {
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		clearClients(fd);
		close(fd);
	} else {
		buff[bytes] = '\0';
		std::cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;
	}
}

void Server::ServerInit()
{
	SerSocket();

	std::cout << GRE << "Server <" << serSocketFd_ << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";

	while (Server::signal_ == false)
	{
		if((poll(&fds_[0],fds_.size(),-1) == -1) && Server::signal_ == false)
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < fds_.size(); i++) {
			if (fds_[i].revents & POLLIN) {
				if (fds_[i].fd == serSocketFd_)
					AcceptNewClient();
				else
					ReceiveNewData(fds_[i].fd);
		}
}
	}
	closeFds();
}
