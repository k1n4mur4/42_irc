#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <vector>

class Client;

class Channel
{
	public:
		Channel();
		Channel(const std::string& name);

		const std::string&	getName() const {return name_;}
		const std::string&	getTopic() const {return topic_;}
		void				setTopic(const std::string& topic){topic_ = topic;}
		const std::string&	getKey() const {return key_;}
		void				setKey(const std::string& key){key_ = key;}
		int					getUserLimit() const {return user_limit_;}
		void				setUserLimit(int limit){user_limit_ = limit;}
		bool				isInviteOnly() const {return invite_only_;}
		void				setInviteOnly(bool v){invite_only_ = v;}
		bool				isTopicRestricted() const {return topic_restricted_;}
		void				setTopicRestricted(bool v){topic_restricted_ = v;}

		void				AddMember(Client* client);
		void				RemoveMember(Client* client);
		bool				IsMember(Client* client) const;
		int					MemberCount() const;
		const std::vector<Client*>&	getMembers() const {return members_;}

		void				AddOperator(Client* client);
		void				RemoveOperator(Client* client);
		bool				IsOperator(Client* client) const;

		void				AddInvited(const std::string& nick);
		bool				IsInvited(const std::string& nick) const;
		void				RemoveInvited(const std::string& nick);

		void				Broadcast(const std::string& message, Client* exclude);

	private:
		std::string				name_;
		std::string				topic_;
		std::string				key_;
		int						user_limit_;
		bool					invite_only_;
		bool					topic_restricted_;
		std::vector<Client*>	members_;
		std::vector<Client*>	operators_;
		std::vector<std::string>	invited_;
};

#endif
