#include "ft_irc.hpp"

void	Server::clearClients(int fd){
	for(size_t i = 0; i < fds_.size(); i++){
		if (fds_[i].fd == fd) {
			fds_.erase(fds_.begin() + i);
			break;
		}
	}
	clients_.erase(fd);
}

void	Server::signalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::signal_ = true;
}

void	Server::closeFds(){
	for (std::map<int, Client>::iterator it = clients_.begin(); it != clients_.end(); ++it){
		std::cout << RED << "Client <" << it->first << "> Disconnected" << WHI << std::endl;
		close(it->first);
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
		throw(std::runtime_error("failed to create socket"));

	int en = 1;
	if(setsockopt(serSocketFd_, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
	if (fcntl(serSocketFd_, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("failed to set option (O_NONBLOCK) on socket"));
	if (bind(serSocketFd_, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("failed to bind socket"));
	if (listen(serSocketFd_, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() failed"));

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
		close(incofd);
		return;
	}

	NewPoll.fd = incofd;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;

	cli.setFd(incofd);
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
	clients_[incofd] = cli;
	fds_.push_back(NewPoll);

	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::ReceiveNewData(int fd)
{
	char buff[1024];

	memset(buff, 0, sizeof(buff));

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0) {
		DisconnectClient(fd);
		return;
	}

	buff[bytes] = '\0';
	std::cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;

	Client* client = FindClientByFd(fd);
	if (!client)
		return;

	client->appendRecvBuffer(std::string(buff, bytes));
	while (client->hasCompleteLine()) {
		std::string line = client->extractLine();
		if (line.empty())
			continue;
		IRCMessage msg = Message::parse(line);
		Command::execute(*this, *client, msg);
	}
}

void Server::HandlePollout(int fd)
{
	Client* client = FindClientByFd(fd);
	if (!client || !client->hasPendingData())
		return;

	const std::string& buf = client->getSendBuffer();
	ssize_t sent = send(fd, buf.c_str(), buf.size(), 0);
	if (sent > 0)
		client->eraseSendBuffer(static_cast<size_t>(sent));
}

void Server::SendToClient(int fd, const std::string& message)
{
	Client* client = FindClientByFd(fd);
	if (!client)
		return;
	client->appendSendBuffer(message);
}

Client*	Server::FindClientByFd(int fd)
{
	std::map<int, Client>::iterator it = clients_.find(fd);
	if (it == clients_.end())
		return NULL;
	return &it->second;
}

Client*	Server::FindClientByNick(const std::string& nick)
{
	for (std::map<int, Client>::iterator it = clients_.begin(); it != clients_.end(); ++it) {
		if (it->second.getNickname() == nick)
			return &it->second;
	}
	return NULL;
}

void	Server::SendReply(Client& client, const std::string& numeric,
						  const std::string& params)
{
	std::string nick = client.getNickname();
	if (nick.empty())
		nick = "*";
	std::string reply = ":" + Global::ServerName + " " + numeric + " " + nick
						+ " " + params + "\r\n";
	SendToClient(client.getFd(), reply);
}

void	Server::DisconnectClient(int fd)
{
	Client* client = FindClientByFd(fd);
	if (client)
		RemoveClientFromAllChannels(client, "Client disconnected");
	std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
	clearClients(fd);
	close(fd);
}

Channel*	Server::FindChannel(const std::string& name)
{
	std::map<std::string, Channel>::iterator it = channels_.find(name);
	if (it == channels_.end())
		return NULL;
	return &it->second;
}

Channel*	Server::FindOrCreateChannel(const std::string& name)
{
	std::map<std::string, Channel>::iterator it = channels_.find(name);
	if (it != channels_.end())
		return &it->second;
	channels_[name] = Channel(name);
	return &channels_[name];
}

void	Server::RemoveChannel(const std::string& name)
{
	channels_.erase(name);
}

void	Server::RemoveClientFromAllChannels(Client* client, const std::string& quit_msg)
{
	std::string msg = ":" + client->getPrefix() + " QUIT :" + quit_msg + "\r\n";
	std::vector<std::string> to_remove;

	for (std::map<std::string, Channel>::iterator it = channels_.begin();
		 it != channels_.end(); ++it) {
		if (it->second.IsMember(client)) {
			it->second.Broadcast(msg, client);
			it->second.RemoveMember(client);
			if (it->second.MemberCount() == 0)
				to_remove.push_back(it->first);
		}
	}
	for (size_t i = 0; i < to_remove.size(); ++i)
		channels_.erase(to_remove[i]);
}

void	Server::BroadcastNickChange(Client* client, const std::string& oldPrefix,
									const std::string& newNick)
{
	std::string msg = ":" + oldPrefix + " NICK " + newNick + "\r\n";
	std::set<int> notified;

	for (std::map<std::string, Channel>::iterator it = channels_.begin();
		 it != channels_.end(); ++it) {
		if (it->second.IsMember(client)) {
			const std::vector<Client*>& members = it->second.getMembers();
			for (size_t i = 0; i < members.size(); ++i) {
				if (members[i] != client &&
					notified.find(members[i]->getFd()) == notified.end()) {
					SendToClient(members[i]->getFd(), msg);
					notified.insert(members[i]->getFd());
				}
			}
		}
	}
}

void Server::ServerInit()
{
	SerSocket();

	std::cout << GRE << "Server <" << serSocketFd_ << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";

	while (Server::signal_ == false)
	{
		// Set POLLOUT for clients with pending data
		for (size_t i = 0; i < fds_.size(); i++) {
			if (fds_[i].fd != serSocketFd_) {
				Client* c = FindClientByFd(fds_[i].fd);
				if (c && c->hasPendingData())
					fds_[i].events = POLLIN | POLLOUT;
				else
					fds_[i].events = POLLIN;
			}
		}

		if((poll(&fds_[0],fds_.size(),-1) == -1) && Server::signal_ == false)
			throw(std::runtime_error("poll() failed"));

		for (size_t i = 0; i < fds_.size(); i++) {
			if (fds_[i].revents & POLLIN) {
				if (fds_[i].fd == serSocketFd_)
					AcceptNewClient();
				else
					ReceiveNewData(fds_[i].fd);
			}
			if (i < fds_.size() && (fds_[i].revents & POLLOUT)) {
				HandlePollout(fds_[i].fd);
			}
		}
	}
	closeFds();
}
