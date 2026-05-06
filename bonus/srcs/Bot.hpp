#ifndef BOT_HPP
# define BOT_HPP

#include <string>

class Bot {

	private:
		std::string	_host;
		std::string	_port;
		std::string	_server_password;
		int			_socket_fd;
		std::string	_recv_buffer;

		void		send_message(std::string msg);
		void		connected();
		void		disconnected();
		void		send_time(const std::string& reply_to);
		void		handle_line(const std::string& line);
		void		handle_privmsg(const std::string& sender,
									const std::string& target,
									const std::string& message);
		std::string	current_time_string() const;

	public:
		Bot(std::string host, std::string port, std::string server_password);
		~Bot();

		void	bot_connect();
		bool	tick();
};

#endif
