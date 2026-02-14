#ifndef COMMAND_HPP
# define COMMAND_HPP

#include "message.hpp"

class Server;
class Client;

namespace Command {
	void execute(Server& server, Client& client, const IRCMessage& msg);
}

#endif
