#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

class Client
{
	public:
		Client(){fd_ = -1; pass_received_ = false; registered_ = false;}
		int		getFd(){return fd_;}
		void	setFd(int fd){fd_ = fd;}

		std::string	getIpAdd() {return ip_add_;}
		void		setIpAdd(std::string ipadd){ip_add_ = ipadd;}

		void		appendRecvBuffer(const std::string& data);
		bool		hasCompleteLine() const;
		std::string	extractLine();

		void		appendSendBuffer(const std::string& data);
		const std::string&	getSendBuffer() const;
		void		eraseSendBuffer(size_t n);
		bool		hasPendingData() const;

		std::string	getNickname() const {return nickname_;}
		void		setNickname(const std::string& nick){nickname_ = nick;}
		std::string	getUsername() const {return username_;}
		void		setUsername(const std::string& user){username_ = user;}
		std::string	getRealname() const {return realname_;}
		void		setRealname(const std::string& real){realname_ = real;}

		bool		isPassReceived() const {return pass_received_;}
		void		setPassReceived(bool v){pass_received_ = v;}
		bool		isRegistered() const {return registered_;}
		void		setRegistered(bool v){registered_ = v;}

		std::string	getPrefix() const;

	private:
		int			fd_;
		std::string	ip_add_;
		std::string	recv_buffer_;
		std::string	send_buffer_;

		std::string	nickname_;
		std::string	username_;
		std::string	realname_;
		bool		pass_received_;
		bool		registered_;
};

#endif
