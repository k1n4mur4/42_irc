#include "client.hpp"

void	Client::appendRecvBuffer(const std::string& data)
{
	recv_buffer_ += data;
}

bool	Client::hasCompleteLine() const
{
	return recv_buffer_.find('\n') != std::string::npos;
}

std::string	Client::extractLine()
{
	std::string::size_type pos = recv_buffer_.find('\n');
	if (pos == std::string::npos)
		return "";
	std::string line = recv_buffer_.substr(0, pos);
	recv_buffer_.erase(0, pos + 1);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	return line;
}

void	Client::appendSendBuffer(const std::string& data)
{
	send_buffer_ += data;
}

const std::string&	Client::getSendBuffer() const
{
	return send_buffer_;
}

void	Client::eraseSendBuffer(size_t n)
{
	send_buffer_.erase(0, n);
}

bool	Client::hasPendingData() const
{
	return !send_buffer_.empty();
}

std::string	Client::getPrefix() const
{
	return nickname_ + "!" + username_ + "@" + ip_add_;
}
