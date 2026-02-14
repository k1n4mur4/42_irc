#ifndef MESSAGE_HPP
# define MESSAGE_HPP

#include <string>
#include <vector>

struct IRCMessage {
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
};

namespace Message {
	IRCMessage parse(const std::string& line);
	std::string build(const std::string& prefix,
					  const std::string& command,
					  const std::vector<std::string>& params);
}

#endif
