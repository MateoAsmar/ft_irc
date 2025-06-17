#include "commands.hpp"
#include "util.hpp"
#include "state.hpp"

// JOIN <#channel>[,#other] [key[,key2]]
void cmd_JOIN(Client* c, const std::vector<std::string>& p)
{
	if (p.size() < 2)
	{
		send_err(c, "461", "JOIN", "Not enough parameters");
		std::cout << "JOIN: Not enough parameters for client " << c->fd << "\n";
		return;
	}

	std::vector<std::string> chans = split(p[1], ',');
	std::vector<std::string> keys;

	if (p.size() > 2)
		keys = split(p[2], ',');

	for (size_t i = 0; i < chans.size(); ++i)
	{
		const std::string& chanName = chans[i];
		std::string key;

		if (i < keys.size())
			key = keys[i];
		else
			key = "";

		// Reject if the provided key contains any whitespace (spaces, tabs, etc.)
		for (size_t k = 0; k < key.size(); ++k)
		{
			if (std::isspace(static_cast<unsigned char>(key[k])))
			{
				send_err(c, "475", chanName, "Invalid channel key");
				std::cout << "JOIN: Channel key ivalid for client " << c->fd << " on " << chanName << "\n";
				// Skip attempting to join this channel
				continue;
			}
		}

		Channel* ch = get_chan(chanName);

		// check +i (invite‐only)
		if (ch->invite_only && !ch->invited.count(c->nick) && !ch->operators.count(c->nick))
		{
			send_err(c, "473", chanName, "Cannot join channel (+i)");
			std::cout << "JOIN: Client " << c->fd << " cannot join +i channel " << chanName << "\n";
			continue;
		}

		// check +k (key)
		// enforce +k: must supply exactly one token that matches the channel key
	if (!ch->key.empty()) {
		// missing key entirely
		if (key.empty()) {
			send_err(c, "475", chanName, "Cannot join channel (+k)");
			std::cout << "JOIN: missing key for client " << c->fd << " on " << chanName << "\n";
			continue;
		}

		// wrong key or extra tokens (p.size()>3 means there was something after the key)
		if (key != ch->key || p.size() > 3) {
			send_err(c, "475", chanName, "Key is incorrect");
			std::cout << "JOIN: incorrect key or extra tokens for client "
					<< c->fd << " on " << chanName << "\n";
			continue;
		}

		// otherwise key matches exactly and there's no extra token—allow join
	}

		// check +l (limit)
		if (ch->limit > 0 && static_cast<int>(ch->members.size()) >= ch->limit)
		{
			send_err(c, "471", chanName, "Cannot join channel (+l)");
			std::cout << "JOIN: Client " << c->fd << " channel " << chanName << " is full\n";
			continue;
		}

		// actually add the client
		ch->members.push_back(c);
		if (ch->members.size() == 1)
		{
			ch->operators.insert(c->nick);
		}

		// broadcast JOIN line
		{
			std::string joinMsg = ":" + c->nick + "!" + c->user + "@" + server_name + " JOIN " + chanName;
			for (size_t j = 0; j < ch->members.size(); ++j)
				queue_raw(ch->members[j], joinMsg);
		}

		// send topic (331/332)
		if (ch->topic.empty())
			queue_raw(c, ":" + server_name + " 331 " + c->nick + " " + chanName + " :No topic is set");
		else
			queue_raw(c, ":" + server_name + " 332 " + c->nick + " " + chanName + " :" + ch->topic);

		// send NAMES (353) + 366
		{
			std::ostringstream ns;
			for (size_t j = 0; j < ch->members.size(); ++j)
			{
				if (j)
					ns << " ";
				if (ch->operators.count(ch->members[j]->nick))
					ns << "@" << ch->members[j]->nick;
				else
					ns << ch->members[j]->nick;
			}
			queue_raw(c, ":" + server_name + " 353 " + c->nick + " = " + chanName + " :" + ns.str());
			queue_raw(c, ":" + server_name + " 366 " + c->nick + " " + chanName + " :End of /NAMES list");
		}

		// “* Invite mode: +i” or “* Invite mode: -i”
		{
			std::string inviteMode = (ch->invite_only ? "+i" : "-i");
			std::string inviteNotice = ":" + server_name + " NOTICE " + c->nick + " :* Invite mode: " + inviteMode;
			queue_raw(c, inviteNotice);
		}

		// “* Key mode: +k <key>” or “* Key mode: -k”
		{
			std::string keyMode;
			if (!ch->key.empty())
			{
				keyMode = "+k ";
				keyMode += ch->key;
			}
			else
				keyMode = "-k";
			std::string keyNotice = ":" + server_name + " NOTICE " + c->nick + " :* Key mode: " + keyMode;
			queue_raw(c, keyNotice);
		}

		// “* limit is : <limit>” or “* limit is : no limit”
		{
			std::string limitStr;
			if (ch->limit > 0)
			{
				std::ostringstream os;
				os << ch->limit;
				limitStr = os.str();
			}
			else
				limitStr = "no limit";
			std::string limitNotice = ":" + server_name + " NOTICE " + c->nick + " :* limit is : " + limitStr;
			queue_raw(c, limitNotice);
		}

		// “* TOPIC LOCKED: YES” or “* TOPIC LOCKED: NO”
		{
			std::string topicLocked = (ch->topic_locked ? "YES" : "NO");
			std::string topicNotice = ":" + server_name + " NOTICE " + c->nick + " :* TOPIC LOCKED: " + topicLocked;
			queue_raw(c, topicNotice);
		}

		// send current channel‐mode (+i/+t/+k/+l) to the joining user
		{
			std::string modeFlags = "+";
			std::ostringstream params;

			if (ch->invite_only)
				modeFlags += "i";
			if (ch->topic_locked)
				modeFlags += "t";
			if (!ch->key.empty())
			{
				modeFlags += "k";
				params << " " << ch->key;
			}
			if (ch->limit > 0)
			{
				modeFlags += "l";
				params << " " << ch->limit;
			}

			// Only send if at least one flag is set
			if (modeFlags.size() > 1)
			{
				std::string modeMsg = ":" + server_name + " MODE " + chanName + " " + modeFlags + params.str();
				queue_raw(c, modeMsg);
				std::cout << "JOIN: sent modes " << modeFlags << " to client " << c->fd
						  << " on channel " << chanName << "\n";
			}
		}

		std::cout << "Client " << c->fd << " joined channel " << chanName << "\n";

		continue;
	}
}
