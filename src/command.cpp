#include "ft_irc.hpp"
#include <cctype>
#include <cstdlib>

static bool isValidNick(const std::string& nick)
{
	if (nick.empty() || nick.size() > 9)
		return false;
	if (!std::isalpha(nick[0]) && nick[0] != '_')
		return false;
	for (size_t i = 1; i < nick.size(); ++i) {
		char c = nick[i];
		if (!std::isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']'
			&& c != '\\' && c != '`' && c != '^' && c != '{' && c != '}')
			return false;
	}
	return true;
}

static void tryRegister(Server& server, Client& client)
{
	if (client.isRegistered())
		return;
	if (!client.isPassReceived() || client.getNickname().empty()
		|| client.getUsername().empty())
		return;
	client.setRegistered(true);
	server.SendReply(client, "001",
		":Welcome to the Internet Relay Chat Network " + client.getPrefix());
	server.SendReply(client, "002",
		":Your host is " + Global::ServerName + ", running version " + Global::Version);
	server.SendReply(client, "003",
		":This server was created today");
	server.SendReply(client, "004",
		Global::ServerName + " " + Global::Version + " o itkol");
}

// ============================================================
// Pre-registration commands
// ============================================================

static void cmd_pass(Server& server, Client& client, const IRCMessage& msg)
{
	if (client.isRegistered()) {
		server.SendReply(client, "462", ":You may not reregister");
		return;
	}
	if (msg.params.empty()) {
		server.SendReply(client, "461", "PASS :Not enough parameters");
		return;
	}
	if (msg.params[0] != server.getPassword()) {
		server.SendReply(client, "464", ":Password incorrect");
		return;
	}
	client.setPassReceived(true);
	tryRegister(server, client);
}

static void cmd_nick(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.params.empty()) {
		server.SendReply(client, "431", ":No nickname given");
		return;
	}
	std::string nick = msg.params[0];
	if (!isValidNick(nick)) {
		server.SendReply(client, "432", nick + " :Erroneous nickname");
		return;
	}
	Client* existing = server.FindClientByNick(nick);
	if (existing && existing->getFd() != client.getFd()) {
		server.SendReply(client, "433", nick + " :Nickname is already in use");
		return;
	}
	std::string oldPrefix = client.getPrefix();
	std::string oldNick = client.getNickname();
	client.setNickname(nick);
	if (client.isRegistered()) {
		std::string nick_msg = ":" + oldPrefix + " NICK " + nick + "\r\n";
		server.SendToClient(client.getFd(), nick_msg);
		server.BroadcastNickChange(&client, oldPrefix, nick);
	}
	tryRegister(server, client);
}

static void cmd_user(Server& server, Client& client, const IRCMessage& msg)
{
	if (client.isRegistered()) {
		server.SendReply(client, "462", ":You may not reregister");
		return;
	}
	if (msg.params.size() < 4) {
		server.SendReply(client, "461", "USER :Not enough parameters");
		return;
	}
	client.setUsername(msg.params[0]);
	client.setRealname(msg.params[3]);
	tryRegister(server, client);
}

static void cmd_quit(Server& server, Client& client, const IRCMessage& msg)
{
	std::string reason = "Quit";
	if (!msg.params.empty())
		reason = msg.params[0];

	std::string error_msg = "ERROR :Closing Link: " + client.getIpAdd()
							+ " (" + reason + ")\r\n";
	server.SendToClient(client.getFd(), error_msg);
	server.DisconnectClient(client.getFd());
}

static void cmd_ping(Server& server, Client& client, const IRCMessage& msg)
{
	std::string token;
	if (!msg.params.empty())
		token = msg.params[0];
	std::vector<std::string> params;
	params.push_back(token);
	server.SendToClient(client.getFd(),
		Message::build(Global::ServerName, "PONG", params));
}

// ============================================================
// Channel commands
// ============================================================

static void sendNamesReply(Server& server, Client& client, Channel& channel)
{
	std::string names;
	const std::vector<Client*>& members = channel.getMembers();
	for (size_t i = 0; i < members.size(); ++i) {
		if (!names.empty())
			names += " ";
		if (channel.IsOperator(members[i]))
			names += "@";
		names += members[i]->getNickname();
	}
	server.SendReply(client, "353", "= " + channel.getName() + " :" + names);
	server.SendReply(client, "366", channel.getName() + " :End of /NAMES list");
}

static void cmd_join(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.params.empty()) {
		server.SendReply(client, "461", "JOIN :Not enough parameters");
		return;
	}

	std::string channels_str = msg.params[0];
	std::string keys_str;
	if (msg.params.size() > 1)
		keys_str = msg.params[1];

	// Split channels and keys by comma
	std::vector<std::string> chan_list;
	std::vector<std::string> key_list;
	{
		std::string::size_type pos;
		std::string tmp = channels_str;
		while ((pos = tmp.find(',')) != std::string::npos) {
			chan_list.push_back(tmp.substr(0, pos));
			tmp = tmp.substr(pos + 1);
		}
		if (!tmp.empty())
			chan_list.push_back(tmp);

		tmp = keys_str;
		while ((pos = tmp.find(',')) != std::string::npos) {
			key_list.push_back(tmp.substr(0, pos));
			tmp = tmp.substr(pos + 1);
		}
		if (!tmp.empty())
			key_list.push_back(tmp);
	}

	for (size_t i = 0; i < chan_list.size(); ++i) {
		std::string name = chan_list[i];
		if (name.empty() || (name[0] != '#' && name[0] != '&')) {
			server.SendReply(client, "403", name + " :No such channel");
			continue;
		}

		Channel* chan = server.FindChannel(name);
		bool is_new = (chan == NULL);

		if (!is_new) {
			if (chan->IsMember(&client))
				continue;
			if (chan->isInviteOnly() && !chan->IsInvited(client.getNickname())) {
				server.SendReply(client, "473", name + " :Cannot join channel (+i)");
				continue;
			}
			if (!chan->getKey().empty()) {
				std::string key;
				if (i < key_list.size())
					key = key_list[i];
				if (key != chan->getKey()) {
					server.SendReply(client, "475", name + " :Cannot join channel (+k)");
					continue;
				}
			}
			if (chan->getUserLimit() > 0 && chan->MemberCount() >= chan->getUserLimit()) {
				server.SendReply(client, "471", name + " :Cannot join channel (+l)");
				continue;
			}
		}

		if (is_new)
			chan = server.FindOrCreateChannel(name);

		chan->AddMember(&client);
		chan->RemoveInvited(client.getNickname());

		if (is_new)
			chan->AddOperator(&client);

		std::string join_msg = ":" + client.getPrefix() + " JOIN " + name + "\r\n";
		chan->Broadcast(join_msg, NULL);

		if (!chan->getTopic().empty())
			server.SendReply(client, "332", name + " :" + chan->getTopic());
		else
			server.SendReply(client, "331", name + " :No topic is set");

		sendNamesReply(server, client, *chan);
	}
}

static void cmd_part(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.params.empty()) {
		server.SendReply(client, "461", "PART :Not enough parameters");
		return;
	}

	std::string reason;
	if (msg.params.size() > 1)
		reason = msg.params[1];

	std::vector<std::string> chan_list;
	{
		std::string tmp = msg.params[0];
		std::string::size_type pos;
		while ((pos = tmp.find(',')) != std::string::npos) {
			chan_list.push_back(tmp.substr(0, pos));
			tmp = tmp.substr(pos + 1);
		}
		if (!tmp.empty())
			chan_list.push_back(tmp);
	}

	for (size_t i = 0; i < chan_list.size(); ++i) {
		std::string name = chan_list[i];
		Channel* chan = server.FindChannel(name);
		if (!chan) {
			server.SendReply(client, "403", name + " :No such channel");
			continue;
		}
		if (!chan->IsMember(&client)) {
			server.SendReply(client, "442", name + " :You're not on that channel");
			continue;
		}

		std::string part_msg = ":" + client.getPrefix() + " PART " + name;
		if (!reason.empty())
			part_msg += " :" + reason;
		part_msg += "\r\n";
		chan->Broadcast(part_msg, NULL);

		chan->RemoveMember(&client);
		if (chan->MemberCount() == 0)
			server.RemoveChannel(name);
	}
}

static void cmd_privmsg(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.params.empty()) {
		server.SendReply(client, "411", ":No recipient given (PRIVMSG)");
		return;
	}
	if (msg.params.size() < 2) {
		server.SendReply(client, "412", ":No text to send");
		return;
	}

	std::string target = msg.params[0];
	std::string text = msg.params[1];
	std::string privmsg = ":" + client.getPrefix() + " PRIVMSG " + target + " :" + text + "\r\n";

	if (target[0] == '#' || target[0] == '&') {
		Channel* chan = server.FindChannel(target);
		if (!chan) {
			server.SendReply(client, "403", target + " :No such channel");
			return;
		}
		if (!chan->IsMember(&client)) {
			server.SendReply(client, "404", target + " :Cannot send to channel");
			return;
		}
		chan->Broadcast(privmsg, &client);
	} else {
		Client* dest = server.FindClientByNick(target);
		if (!dest) {
			server.SendReply(client, "401", target + " :No such nick/channel");
			return;
		}
		server.SendToClient(dest->getFd(), privmsg);
	}
}

// ============================================================
// Operator commands
// ============================================================

static void cmd_kick(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.params.size() < 2) {
		server.SendReply(client, "461", "KICK :Not enough parameters");
		return;
	}
	std::string chan_name = msg.params[0];
	std::string target_nick = msg.params[1];
	std::string reason = client.getNickname();
	if (msg.params.size() > 2)
		reason = msg.params[2];

	Channel* chan = server.FindChannel(chan_name);
	if (!chan) {
		server.SendReply(client, "403", chan_name + " :No such channel");
		return;
	}
	if (!chan->IsMember(&client)) {
		server.SendReply(client, "442", chan_name + " :You're not on that channel");
		return;
	}
	if (!chan->IsOperator(&client)) {
		server.SendReply(client, "482", chan_name + " :You're not channel operator");
		return;
	}
	Client* target = server.FindClientByNick(target_nick);
	if (!target || !chan->IsMember(target)) {
		server.SendReply(client, "441", target_nick + " " + chan_name
						 + " :They aren't on that channel");
		return;
	}

	std::string kick_msg = ":" + client.getPrefix() + " KICK " + chan_name
						   + " " + target_nick + " :" + reason + "\r\n";
	chan->Broadcast(kick_msg, NULL);
	chan->RemoveMember(target);
	if (chan->MemberCount() == 0)
		server.RemoveChannel(chan_name);
}

static void cmd_invite(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.params.size() < 2) {
		server.SendReply(client, "461", "INVITE :Not enough parameters");
		return;
	}
	std::string target_nick = msg.params[0];
	std::string chan_name = msg.params[1];

	Channel* chan = server.FindChannel(chan_name);
	if (!chan) {
		server.SendReply(client, "403", chan_name + " :No such channel");
		return;
	}
	if (!chan->IsMember(&client)) {
		server.SendReply(client, "442", chan_name + " :You're not on that channel");
		return;
	}
	if (chan->isInviteOnly() && !chan->IsOperator(&client)) {
		server.SendReply(client, "482", chan_name + " :You're not channel operator");
		return;
	}
	Client* target = server.FindClientByNick(target_nick);
	if (!target) {
		server.SendReply(client, "401", target_nick + " :No such nick/channel");
		return;
	}
	if (chan->IsMember(target)) {
		server.SendReply(client, "443", target_nick + " " + chan_name
						 + " :is already on channel");
		return;
	}

	chan->AddInvited(target_nick);
	server.SendReply(client, "341", target_nick + " " + chan_name);
	std::string invite_msg = ":" + client.getPrefix() + " INVITE " + target_nick
							 + " " + chan_name + "\r\n";
	server.SendToClient(target->getFd(), invite_msg);
}

static void cmd_topic(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.params.empty()) {
		server.SendReply(client, "461", "TOPIC :Not enough parameters");
		return;
	}
	std::string chan_name = msg.params[0];
	Channel* chan = server.FindChannel(chan_name);
	if (!chan) {
		server.SendReply(client, "403", chan_name + " :No such channel");
		return;
	}
	if (!chan->IsMember(&client)) {
		server.SendReply(client, "442", chan_name + " :You're not on that channel");
		return;
	}

	if (msg.params.size() < 2) {
		if (chan->getTopic().empty())
			server.SendReply(client, "331", chan_name + " :No topic is set");
		else
			server.SendReply(client, "332", chan_name + " :" + chan->getTopic());
		return;
	}

	if (chan->isTopicRestricted() && !chan->IsOperator(&client)) {
		server.SendReply(client, "482", chan_name + " :You're not channel operator");
		return;
	}

	chan->setTopic(msg.params[1]);
	std::string topic_msg = ":" + client.getPrefix() + " TOPIC " + chan_name
							+ " :" + msg.params[1] + "\r\n";
	chan->Broadcast(topic_msg, NULL);
}

static void cmd_mode(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.params.empty()) {
		server.SendReply(client, "461", "MODE :Not enough parameters");
		return;
	}
	std::string target = msg.params[0];

	// User mode query — ignore silently
	if (target[0] != '#' && target[0] != '&')
		return;

	Channel* chan = server.FindChannel(target);
	if (!chan) {
		server.SendReply(client, "403", target + " :No such channel");
		return;
	}

	// Mode query: reply with current modes
	if (msg.params.size() < 2) {
		std::string modes = "+";
		std::string mode_params;
		if (chan->isInviteOnly())
			modes += "i";
		if (chan->isTopicRestricted())
			modes += "t";
		if (!chan->getKey().empty()) {
			modes += "k";
			mode_params += " " + chan->getKey();
		}
		if (chan->getUserLimit() > 0) {
			modes += "l";
			std::ostringstream oss;
			oss << chan->getUserLimit();
			mode_params += " " + oss.str();
		}
		if (modes == "+")
			modes = "+";
		server.SendReply(client, "324", target + " " + modes + mode_params);
		return;
	}

	if (!chan->IsMember(&client)) {
		server.SendReply(client, "442", target + " :You're not on that channel");
		return;
	}
	if (!chan->IsOperator(&client)) {
		server.SendReply(client, "482", target + " :You're not channel operator");
		return;
	}

	std::string mode_str = msg.params[1];
	size_t param_idx = 2;
	bool adding = true;

	std::string applied_modes;
	std::string applied_params;
	bool last_was_plus = true;

	for (size_t i = 0; i < mode_str.size(); ++i) {
		char c = mode_str[i];
		if (c == '+') {
			adding = true;
			continue;
		}
		if (c == '-') {
			adding = false;
			continue;
		}

		switch (c) {
			case 'i':
				chan->setInviteOnly(adding);
				if (adding != last_was_plus || applied_modes.empty()) {
					applied_modes += adding ? "+" : "-";
					last_was_plus = adding;
				}
				applied_modes += "i";
				break;
			case 't':
				chan->setTopicRestricted(adding);
				if (adding != last_was_plus || applied_modes.empty()) {
					applied_modes += adding ? "+" : "-";
					last_was_plus = adding;
				}
				applied_modes += "t";
				break;
			case 'k':
				if (adding) {
					if (param_idx >= msg.params.size()) {
						server.SendReply(client, "461", "MODE :Not enough parameters");
						continue;
					}
					chan->setKey(msg.params[param_idx]);
					if (adding != last_was_plus || applied_modes.empty()) {
						applied_modes += "+";
						last_was_plus = true;
					}
					applied_modes += "k";
					applied_params += " " + msg.params[param_idx];
					param_idx++;
				} else {
					chan->setKey("");
					if (adding != last_was_plus || applied_modes.empty()) {
						applied_modes += "-";
						last_was_plus = false;
					}
					applied_modes += "k";
				}
				break;
			case 'o': {
				if (param_idx >= msg.params.size()) {
					server.SendReply(client, "461", "MODE :Not enough parameters");
					continue;
				}
				std::string nick = msg.params[param_idx];
				param_idx++;
				Client* tgt = server.FindClientByNick(nick);
				if (!tgt || !chan->IsMember(tgt)) {
					server.SendReply(client, "441",
						nick + " " + target + " :They aren't on that channel");
					continue;
				}
				if (adding)
					chan->AddOperator(tgt);
				else
					chan->RemoveOperator(tgt);
				if (adding != last_was_plus || applied_modes.empty()) {
					applied_modes += adding ? "+" : "-";
					last_was_plus = adding;
				}
				applied_modes += "o";
				applied_params += " " + nick;
				break;
			}
			case 'l':
				if (adding) {
					if (param_idx >= msg.params.size()) {
						server.SendReply(client, "461", "MODE :Not enough parameters");
						continue;
					}
					int limit = std::atoi(msg.params[param_idx].c_str());
					param_idx++;
					if (limit <= 0)
						continue;
					chan->setUserLimit(limit);
					if (adding != last_was_plus || applied_modes.empty()) {
						applied_modes += "+";
						last_was_plus = true;
					}
					applied_modes += "l";
					std::ostringstream oss;
					oss << limit;
					applied_params += " " + oss.str();
				} else {
					chan->setUserLimit(0);
					if (adding != last_was_plus || applied_modes.empty()) {
						applied_modes += "-";
						last_was_plus = false;
					}
					applied_modes += "l";
				}
				break;
			default:
				server.SendReply(client, "472",
					std::string(1, c) + " :is unknown mode char to me");
				break;
		}
	}

	if (!applied_modes.empty()) {
		std::string mode_msg = ":" + client.getPrefix() + " MODE " + target
							   + " " + applied_modes + applied_params + "\r\n";
		chan->Broadcast(mode_msg, NULL);
	}
}

// ============================================================
// Command dispatcher
// ============================================================

void	Command::execute(Server& server, Client& client, const IRCMessage& msg)
{
	if (msg.command.empty())
		return;

	if (msg.command == "PASS") {
		cmd_pass(server, client, msg);
		return;
	}
	if (msg.command == "NICK") {
		cmd_nick(server, client, msg);
		return;
	}
	if (msg.command == "USER") {
		cmd_user(server, client, msg);
		return;
	}
	if (msg.command == "QUIT") {
		cmd_quit(server, client, msg);
		return;
	}
	if (msg.command == "PING") {
		cmd_ping(server, client, msg);
		return;
	}

	if (!client.isRegistered()) {
		server.SendReply(client, "451", ":You have not registered");
		return;
	}

	if (msg.command == "JOIN") {
		cmd_join(server, client, msg);
		return;
	}
	if (msg.command == "PART") {
		cmd_part(server, client, msg);
		return;
	}
	if (msg.command == "PRIVMSG") {
		cmd_privmsg(server, client, msg);
		return;
	}
	if (msg.command == "KICK") {
		cmd_kick(server, client, msg);
		return;
	}
	if (msg.command == "INVITE") {
		cmd_invite(server, client, msg);
		return;
	}
	if (msg.command == "TOPIC") {
		cmd_topic(server, client, msg);
		return;
	}
	if (msg.command == "MODE") {
		cmd_mode(server, client, msg);
		return;
	}
}
