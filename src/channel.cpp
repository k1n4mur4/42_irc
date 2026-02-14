#include "channel.hpp"
#include "client.hpp"
#include "server.hpp"
#include <algorithm>

Channel::Channel()
	: user_limit_(0), invite_only_(false), topic_restricted_(false) {}

Channel::Channel(const std::string& name)
	: name_(name), user_limit_(0), invite_only_(false), topic_restricted_(false) {}

void	Channel::AddMember(Client* client)
{
	if (!IsMember(client))
		members_.push_back(client);
}

void	Channel::RemoveMember(Client* client)
{
	for (std::vector<Client*>::iterator it = members_.begin(); it != members_.end(); ++it) {
		if (*it == client) {
			members_.erase(it);
			break;
		}
	}
	RemoveOperator(client);
}

bool	Channel::IsMember(Client* client) const
{
	for (size_t i = 0; i < members_.size(); ++i) {
		if (members_[i] == client)
			return true;
	}
	return false;
}

int	Channel::MemberCount() const
{
	return static_cast<int>(members_.size());
}

void	Channel::AddOperator(Client* client)
{
	if (!IsOperator(client))
		operators_.push_back(client);
}

void	Channel::RemoveOperator(Client* client)
{
	for (std::vector<Client*>::iterator it = operators_.begin(); it != operators_.end(); ++it) {
		if (*it == client) {
			operators_.erase(it);
			break;
		}
	}
}

bool	Channel::IsOperator(Client* client) const
{
	for (size_t i = 0; i < operators_.size(); ++i) {
		if (operators_[i] == client)
			return true;
	}
	return false;
}

void	Channel::AddInvited(const std::string& nick)
{
	if (!IsInvited(nick))
		invited_.push_back(nick);
}

bool	Channel::IsInvited(const std::string& nick) const
{
	for (size_t i = 0; i < invited_.size(); ++i) {
		if (invited_[i] == nick)
			return true;
	}
	return false;
}

void	Channel::RemoveInvited(const std::string& nick)
{
	for (std::vector<std::string>::iterator it = invited_.begin(); it != invited_.end(); ++it) {
		if (*it == nick) {
			invited_.erase(it);
			break;
		}
	}
}

void	Channel::Broadcast(const std::string& message, Client* exclude)
{
	for (size_t i = 0; i < members_.size(); ++i) {
		if (members_[i] != exclude)
			members_[i]->appendSendBuffer(message);
	}
}
