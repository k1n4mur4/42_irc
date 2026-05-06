#include <cerrno>
#include <cstring>
#include <ctime>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>


#include "Bot.hpp"


#define BOT_NAME "Attacker"

#define CMD_PASS(password) std::string("PASS ") + password + "\r\n"
#define CMD_NICK(nickname) std::string("NICK ") + nickname + "\r\n"
#define CMD_USER(username, realname) std::string("USER ") + username + " 0 * " + realname + "\r\n"
#define CMD_JOIN(channel) std::string("JOIN ") + channel + "\r\n"
#define CMD_PRIVMSG(target, message) std::string("PRIVMSG ") + target + " :" + std::string(message) + "\r\n"


Bot::Bot(std::string host, std::string port, std::string server_password) {
	this->_host = host;
	this->_port = port;
	this->_server_password = server_password;
	this->_socket_fd = -1;
}

Bot::~Bot() {
}

void Bot::bot_connect() {
	struct addrinfo hints;
	struct addrinfo *addrinfos;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int rv = getaddrinfo(_host.c_str(), _port.c_str(), &hints, &addrinfos);
	if (rv != 0) {
		throw std::runtime_error(gai_strerror(rv));
	}

	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0) {
		freeaddrinfo(addrinfos);
		throw std::runtime_error(strerror(errno));
	}

	if (connect(_socket_fd, addrinfos->ai_addr, addrinfos->ai_addrlen) != 0) {
		freeaddrinfo(addrinfos);
		close(_socket_fd);
		throw std::runtime_error(strerror(errno));
	}
	freeaddrinfo(addrinfos);
	connected();
}

void Bot::send_message(std::string msg) {
	if (send(_socket_fd, msg.c_str(), msg.length(), MSG_DONTWAIT | MSG_NOSIGNAL) < 0) {
		throw std::runtime_error(strerror(errno));
	}
}

void Bot::connected() {
	if (_server_password.empty() == false)
		send_message(CMD_PASS(_server_password));
	send_message(CMD_USER(BOT_NAME, BOT_NAME));
	send_message(CMD_NICK(BOT_NAME));
	send_message(CMD_JOIN("#bot"));
	send_message(CMD_PRIVMSG("#bot", "hellow world"));
}

void Bot::disconnected() {
	std::cout << "Disconnected" << std::endl;
}

void Bot::send_time(const std::string& reply_to) {
	send_message(CMD_PRIVMSG(reply_to,
		std::string("Current time: ") + current_time_string()));
}

// Pull one CRLF-terminated line out of buf. Falls back to bare LF for nc/telnet.
static bool extract_line(std::string& buf, std::string& out) {
	std::string::size_type pos = buf.find("\r\n");
	if (pos == std::string::npos) {
		pos = buf.find('\n');
		if (pos == std::string::npos)
			return false;
		out = buf.substr(0, pos);
		buf.erase(0, pos + 1);
	} else {
		out = buf.substr(0, pos);
		buf.erase(0, pos + 2);
	}
	return true;
}

std::string Bot::current_time_string() const {
	std::time_t now = std::time(NULL);
	std::tm* lt = std::localtime(&now);
	char buf[64];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
	return std::string(buf);
}

void Bot::handle_privmsg(const std::string& sender,
						const std::string& target,
						const std::string& message) {
	// Reply to the channel for channel messages, otherwise back to the sender (DM).
	std::string reply_to = (!target.empty() && target[0] == '#') ? target : sender;

	if (message == "!time") {
		try {
			send_time(reply_to);
		} catch (std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	}
}

void Bot::handle_line(const std::string& line) {
	if (line.empty())
		return;

	std::string sender;
	std::string rest = line;

	// Optional source prefix: ":nick!user@host ..."
	if (rest[0] == ':') {
		std::string::size_type sp = rest.find(' ');
		if (sp == std::string::npos)
			return;
		std::string prefix = rest.substr(1, sp - 1);
		std::string::size_type bang = prefix.find('!');
		sender = (bang == std::string::npos) ? prefix : prefix.substr(0, bang);
		rest.erase(0, sp + 1);
	}

	std::string::size_type sp = rest.find(' ');
	if (sp == std::string::npos)
		return;
	std::string cmd = rest.substr(0, sp);
	rest.erase(0, sp + 1);

	if (cmd == "PRIVMSG") {
		// PRIVMSG <target> :<message>
		std::string::size_type sp2 = rest.find(' ');
		if (sp2 == std::string::npos)
			return;
		std::string target = rest.substr(0, sp2);
		rest.erase(0, sp2 + 1);
		if (rest.empty() || rest[0] != ':')
			return;
		std::string message = rest.substr(1);

		handle_privmsg(sender, target, message);
	}
	else if (cmd == "PING") {
		send_message(std::string("PONG ") + rest + "\r\n");
	}
}

bool Bot::tick() {
	char	buff[1024];
	int		n = recv(_socket_fd, buff, sizeof(buff), 0);

	if (n == 0) {
		close(_socket_fd);
		disconnected();
		return false;
	}
	if (n < 0) {
		close(_socket_fd);
		disconnected();
		throw std::runtime_error(strerror(errno));
	}

	_recv_buffer.append(buff, n);

	std::string line;
	while (extract_line(_recv_buffer, line)) {
		std::cout << "<< " << line << std::endl;
		try {
			handle_line(line);
		} catch (std::exception& e) {
			std::cerr << "handler error: " << e.what() << std::endl;
		}
	}
	return true;
}
