#include "commands.hpp"
#include "util.hpp"
#include "state.hpp"

// MODE <#channel> [ +/-modes [ params... ] ]
void cmd_MODE(Client* c, const std::vector<std::string>& p)
{
    // If the client sent exactly "MODE #channel" (no mode chars), reply with current modes
    if (p.size() == 2)
	{
        const std::string& chanName = p[1];
        Channel* ch = get_chan(chanName);

        // Verify the client is in the channel
        bool on_chan = false;
        for (size_t i = 0; i < ch->members.size(); ++i)
		{
            if (ch->members[i] == c)
			{
                on_chan = true;
                break;
            }
        }

        if (!on_chan)
		{
            send_err(c, "442", chanName, "You're not on that channel");
            std::cout << "MODE query: client " << c->fd << " not on " << chanName << "\n";
            return;
        }

        // Build a string of all set modes
        std::string modeFlags = "+";
        std::ostringstream paramsOut;

        if (ch->invite_only)
		{
            modeFlags += "i";
        }

        if (ch->topic_locked)
		{
            modeFlags += "t";
        }

        if (!ch->key.empty())
        {
            send_err(c, "467", chanName, "Channel key already set, remove it first");
            std::cout << "MODE: Key already set for client " << c->fd << "\n";
            return;
        }

        if (ch->limit > 0)
		{
            modeFlags += "l";
            paramsOut << " " << ch->limit;
        }

        // If only "+" remains, no modes are set
        if (modeFlags.size() == 1)
            queue_raw(c, ":" + server_name + " 324 " + c->nick + " " + chanName + " +");

		else
            queue_raw(c, ":" + server_name + " 324 " + c->nick + " " + chanName + " " + modeFlags + paramsOut.str());

        std::cout << "MODE query: sent current modes " << modeFlags << " for channel " << chanName << " to client " << c->fd << "\n";
        return;
    }

    // Otherwise, must be "MODE #channel +/-modes [params]"
    if (p.size() < 3)
	{
        send_err(c, "461", "MODE", "Not enough parameters");
        std::cout << "MODE: Not enough parameters for client " << c->fd << "\n";
        return;
    }

    const std::string& chanName = p[1];
    const std::string& modes = p[2];
    Channel* ch = get_chan(chanName);

    // Verify the client is in the channel
    bool on_chan = false;
    for (size_t i = 0; i < ch->members.size(); ++i)
	{
        if (ch->members[i] == c)
		{
            on_chan = true;
            break;
        }
    }

    if (!on_chan)
	{
        send_err(c, "442", chanName, "You're not on that channel");
        std::cout << "MODE: client " << c->fd << " not on " << chanName << "\n";
        return;
    }

    // Verify the client is an operator
    if (!ch->operators.count(c->nick))
	{
        send_err(c, "482", chanName, "You're not channel operator");
        std::cout << "MODE: client " << c->fd << " not operator on " << chanName << "\n";
        return;
    }

    bool add = true;
    size_t argi = 3;
    std::vector<Client*>& mems = ch->members;

    for (size_t i = 0; i < modes.size(); ++i)
	{
        char m = modes[i];

        if (m == '+')
		{
            add = true;
            continue;
        }

        if (m == '-')
		{
            add = false;
            continue;
        }

        std::string modeStr;
        std::string param;

        if (m == 'i')
		{
            ch->invite_only = add;

            if (add)
                modeStr = "+i";
            else
                modeStr = "-i";
        }

        else if (m == 't')
		{
            ch->topic_locked = add;

            if (add)
                modeStr = "+t";
            else
                modeStr = "-t";
        }

else if (m == 'k')
{
    if (add)
    {
        // +k: cannot set a second key until the existing one is removed
        if (!ch->key.empty())
        {
            send_err(c, "467", chanName, "A Key is already set. Remove old key first to set a new one.");
            std::cout << "MODE: key already set for client " << c->fd << "\n";
            return;
        }

        // need exactly one parameter
        if (argi >= p.size())
        {
            send_err(c, "461", "MODE", "Not enough parameters");
            std::cout << "MODE: missing key parameter for client " << c->fd << "\n";
            return;
        }       

        param = p[argi++];
        if (!param.empty() && param[0] == ':')
            param = param.substr(1);

        // no whitespace allowed in the new key
        for (size_t i = 0; i < param.size(); ++i)
        {
            if (std::isspace(static_cast<unsigned char>(param[i])))
            {
                send_err(c, "467", chanName, "Invalid channel key");
                std::cout << "MODE: key contains whitespace for client " << c->fd << "\n";
                return;
            }
        }

        // no extra tokens
        if (argi < p.size())
        {
            send_err(c, "467", chanName, "Invalid channel key");
            std::cout << "MODE: extra tokens after key for client " << c->fd << "\n";
            return;
        }

        ch->key = param;
        modeStr = "+k";
    }
    else
    {
        // need exactly one parameter for -k
        if (argi >= p.size())
        {
            send_err(c, "461", "MODE", "Not enough parameters");
            std::cout << "MODE: missing key parameter for client " << c->fd << "\n";
            return;
        }

        param = p[argi++];
        if (!param.empty() && param[0] == ':')
            param = param.substr(1);

        // no whitespace allowed in the removal key
        for (size_t i = 0; i < param.size(); ++i)
        {
            if (std::isspace(static_cast<unsigned char>(param[i])))
            {
                send_err(c, "467", chanName, "Invalid channel key");
                std::cout << "MODE: key contains whitespace for client " << c->fd << "\n";
                return;
            }
        }

        // no extra tokens
        if (argi < p.size())
        {
            send_err(c, "467", chanName, "Invalid channel key");
            std::cout << "MODE: extra tokens after key for client " << c->fd << "\n";
            return;
        }

        // only allow removal if it matches
        if (param == ch->key)
        {
            ch->key.clear();
            modeStr = "-k";
        }
        else
        {
            send_err(c, "467", chanName, "Key is incorrect");
            std::cout << "MODE: incorrect key for client " << c->fd << "\n";
            return;
        }
    }
}

        else if (m == 'l')
		{
            if (add)
			{
                if (argi < p.size())
				{
                    param = p[argi++];

                    // Validate that param is a positive integer
                    bool validNumber = true;
					
                    for (size_t k = 0; k < param.size(); ++k)
					{
                        if (!std::isdigit(param[k]))
						{
                            validNumber = false;
                            break;
                        }
                    }

                    if (validNumber)
					{
                        int lim = std::atoi(param.c_str());

                        if (lim > 0)
						{
                            ch->limit = lim;
                            modeStr = "+l";
                        }
						else
                            validNumber = false;
                    }

                    if (!validNumber)
					{
                        send_err(c, "461", "MODE", "Invalid parameter");
                        std::cout << "MODE: invalid  parameter for client " << c->fd << "\n";
                        return;
                    }
                }
				else
				{
                    send_err(c, "461", "MODE", "Not enough parameters");
                    std::cout << "MODE: missing parameter for client " << c->fd << "\n";
                    return;
                }
            }
			else
			{
                ch->limit = 0;
                modeStr = "-l";
            }
        }

        else if (m == 'o')
		{
            // +o or -o for a given user
            if (argi < p.size())
			{
                param = p[argi++];

                if (!param.empty() && param[0] == ':')
				{
                    param = param.substr(1);
                }

                // Check that the target user exists
                Client* tgt = find_nick(param);

                if (!tgt)
				{
                    send_err(c, "401", param, "No such nick");
                    std::cout << "MODE: no such nick " << param << " for client " << c->fd << "\n";
                    return;
                }

                // Check that target is on the channel
                bool tgt_on_chan = false;

                for (size_t k = 0; k < ch->members.size(); ++k)
				{
                    if (ch->members[k] == tgt)
					{
                        tgt_on_chan = true;
                        break;
                    }
                }

                if (!tgt_on_chan)
				{
                    send_err(c, "442", chanName, param);
                    std::cout << "MODE: target " << param << " not on channel " << chanName << "\n";
                    return;
                }

                if (add)
				{
                    ch->operators.insert(param);
                    modeStr = "+o";
                }
				else
				{
                    // If removing operator, ensure at least one op remains
                    if (ch->operators.count(param) > 0)
					{
                        ch->operators.erase(param);
                        modeStr = "-o";
                    }
					else
					{
                        // Trying to remove op status from someone who isn't an operator
                        send_err(c, "482", chanName, "User is not an operator");
                        std::cout << "MODE: " << param << " is not op in " << chanName << "\n";
                        return;
                    }
                }
            }
			else
			{
                send_err(c, "461", "MODE", "Not enough parameters");
                std::cout << "MODE: missing user parameter for client " << c->fd << "\n";
                return;
            }
        }

        else
		{
            // unknown mode letter—skip
            continue;
        }

        // Broadcast a MODE line for just this single change
        std::string msg = ":" + c->nick + "!" + c->user + "@" + server_name + " MODE " + chanName + " " + modeStr;

        if (!param.empty())
            msg += " " + param;

		for (size_t j = 0; j < mems.size(); ++j)
		{
            queue_raw(mems[j], msg);
        }

        std::cout << "Client " << c->fd << " (" << c->nick << ") set mode " << modeStr << " on channel " << chanName;

        if (!param.empty())
            std::cout << " param=" << param;

        std::cout << "\n";
    }
}
