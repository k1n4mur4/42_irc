#include "message.hpp"

IRCMessage	Message::parse(const std::string& line)
{
	IRCMessage msg;
	std::string remainder = line;

	if (remainder.empty())
		return msg;

	// Parse prefix
	if (remainder[0] == ':') {
		std::string::size_type sp = remainder.find(' ');
		if (sp == std::string::npos) {
			msg.prefix = remainder.substr(1);
			return msg;
		}
		msg.prefix = remainder.substr(1, sp - 1);
		remainder = remainder.substr(sp + 1);
	}

	// Skip leading spaces
	while (!remainder.empty() && remainder[0] == ' ')
		remainder.erase(0, 1);

	// Parse command
	{
		std::string::size_type sp = remainder.find(' ');
		if (sp == std::string::npos) {
			msg.command = remainder;
			// Uppercase the command
			for (size_t i = 0; i < msg.command.size(); ++i)
				msg.command[i] = std::toupper(msg.command[i]);
			return msg;
		}
		msg.command = remainder.substr(0, sp);
		for (size_t i = 0; i < msg.command.size(); ++i)
			msg.command[i] = std::toupper(msg.command[i]);
		remainder = remainder.substr(sp + 1);
	}

	// Parse params
	while (!remainder.empty()) {
		while (!remainder.empty() && remainder[0] == ' ')
			remainder.erase(0, 1);
		if (remainder.empty())
			break;
		if (remainder[0] == ':') {
			msg.params.push_back(remainder.substr(1));
			break;
		}
		std::string::size_type sp = remainder.find(' ');
		if (sp == std::string::npos) {
			msg.params.push_back(remainder);
			break;
		}
		msg.params.push_back(remainder.substr(0, sp));
		remainder = remainder.substr(sp + 1);
	}

	return msg;
}

std::string	Message::build(const std::string& prefix,
						   const std::string& command,
						   const std::vector<std::string>& params)
{
	std::string result;

	if (!prefix.empty())
		result = ":" + prefix + " ";
	result += command;
	for (size_t i = 0; i < params.size(); ++i) {
		result += " ";
		if (i == params.size() - 1 &&
			(params[i].empty() || params[i].find(' ') != std::string::npos || params[i][0] == ':'))
			result += ":";
		result += params[i];
	}
	result += "\r\n";
	return result;
}
