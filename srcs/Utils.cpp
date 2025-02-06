#include "../includes/Server.hpp"


void Server::processCommand(Client* client, const char* message)
{
	
	const std::string& msg = message;
	std::vector<std::string> vec(getWords_(msg));               // Splitting the message sent by client word by word
	std::vector<std::string>::iterator it = vec.begin();        // Iterating through the vector containing the message  
	std::cout << message;


	while ((it != vec.end()))
	{
		notCommand(*it);

		if (equalStrings(*it, "PASS") && !client->isValid())
		{	
			if ((it + 1) == vec.end())
			{
				const std::string& msg = ERR_PASSWDMISMATCH(*(it + 1));
				command.sendData(client->getClientSock(), msg);
				return; // !! mandatory return statement
			}
			if (equalStrings(getRangeAsString(vec, it + 1, vec.size(), " "), getPasswd()))
			{
				client->setValid(true);
				command.sendData(client->getClientSock(), RPL_WELCOME(client->getNickName(), "IRC"));
			}
		}

		if (equalStrings(*it, "USER") && client->isValid())
		{
			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
				return;
			client->setUserName(*(it + 1));
		}
	

		if (equalStrings(*it, "NICK") && client->isValid())
		{
			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
				return;

			if (nickNameInUse(client, *(it + 1)))
			{
				const std::string &msg = "IRC :" + ERR_NICKNAMEINUSE(client->getNickName(), *(it + 1));
				command.sendData(client->getClientSock(), msg);
				return;
			}
			if (command.invalidNickName(*(it + 1)))
			{
				const std::string &msg = "IRC :" + ERR_BADNICKNAME(*(it + 1));
				command.sendData(client->getClientSock(), msg);
				return;
			}
			client->setNickName(*(it + 1));
			const std::string& msg = ":" + client->getNickName() + "!" + client->getUserName() + "@" + client->getIpAddress() + " NICK " + ":" + client->getNickName() + "\r\n";
			command.sendData(client->getClientSock(), msg);		
		}

		if (equalStrings(*it, "JOIN") && client->isEligible())
		{
			Channel *chan = NULL;
			const std::string& name = *(it + 1);
			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
				return;
			if (name[0] != '#' || name.size() == 1)
			{
				const std::string& msg = ERR_NEEDMOREPARAMS(client->getNickName(), "JOIN") ;
				command.sendData(client->getClientSock(), msg);
				return;
			}
			chan = command.getChannelByName(name);
			if (chan != NULL)
			{
				if (chan->getPasswdRequired())
				{
					if ((it + 2) == vec.end())
					{
						command.sendData(client->getClientSock(), ERR_CHANHASPASS(chan->getChannelName()));
						return;
					}
				}
			}
			command.joinCommand(client, name, *(it + 2));
		}

		if (equalStrings(*it, "PRIVMSG") && client->isEligible())
		{
			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it))
				|| emptyParam(vec, (it + 2), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
				return;
			
			const std::string& param = *(it + 1);
			if (param.size() <= 1)
			{
				const std::string& msg =  ":" + client->getNickName() + "!" + client->getUserName() + "@" + client->getIpAddress() + ".IP PRIVMSG " + *(it + 1) + " " + ERR_NOSUCHCHANNEL(client->getNickName(), *(it + 1));
				command.sendData(client->getClientSock(), msg);
				return; //!!
			}

			if (param[0] == '#') // channel's name
			{
				if ((it + 2) != vec.end() && command.getChannelByName(param) != NULL)
					command.privmsgCommandChannel(param, client, getRangeAsString(vec, it + 2, vec.size(), " "));
				else if (command.getChannelByName(param) == NULL)
				{
					const std::string& msg =  ":" + client->getNickName() + "!" + client->getUserName() + "@" + client->getIpAddress() + ".IP PRIVMSG " + *(it + 1) + " " + ERR_NOSUCHCHANNEL(client->getNickName(), param);
					command.sendData(client->getClientSock(), msg);
				}
				else
				{
					const std::string& msg =  ":" + client->getNickName() + "!" + client->getUserName() + "@" + client->getIpAddress() + ".IP PRIVMSG " + *(it + 1) + " " + ERR_NEEDMOREPARAMS(client->getNickName(), *it);
					command.sendData(client->getClientSock(), msg);
				}
				return; //!!
			}
			Client *_client = getClientFromServer(param);               //client to whom the msg will be sent
			if(_client != NULL)
			{
				if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it))
					|| emptyParam(vec, (it + 2), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
				std::cout << "sock ->" << _client->getClientSock() << "\n";
				if (getRangeAsString(vec, it + 2, vec.size(), " ").size() > 0)
					command.privmsgCommandUser(client, _client, getRangeAsString(vec, it + 2, vec.size(), " "));
				return; //!!
			}
			else
			{
				command.sendData(client->getClientSock(), ERR_NOSUCHNICK(client->getNickName(), param));
				return ;
			}
		}

		/* I should check if the client is trying to join a channel, where he is already a member in */
		/* To do so i should search in the client vector of that channel and see if the number of the client's sock is aready there
			*/

		if (equalStrings(*it, "MODE") && client->isEligible())
		{
			const std::string& name = *(it + 1);
			if ((it + 1) == vec.end() || !command.channelExist(name))
				return;

			if (vec.size() == 2 && command.channelExist(name))
			{
				const std::string& msg = ":IRC " + RPL_CHANNELMODEIS(client->getNickName(), name, command.getChannelByName(name)->getChannelMode());
           		send(client->getClientSock(), msg.c_str(), msg.size(), 0);
           		const std::string& msg1 = ":IRC " + RPL_CREATIONTIME(client->getNickName(), *(it + 1), std::to_string(command.getChannelByName(name)->getCurrentTimestamp()));
           		send(client->getClientSock(), msg1.c_str(), msg1.size(), 0);
           		return;

			}

			if (((name[0] != '#' || name.size() == 1)) && ((it + 1) != vec.end() && (it + 2) != vec.end()) && command.channelExist(*(it + 1)))
			{
				const std::string& msg = ":IRC " + ERR_NEEDMOREPARAMS(client->getNickName(), "JOIN") ;
				command.sendData(client->getClientSock(), msg);
				return;
			}
		
			else if ((equalStrings(*(it + 2), "+k") || equalStrings(*(it + 2), "+o") || equalStrings(*(it + 2), "+l") || equalStrings(*(it + 2), "-o")) && (it + 3) != vec.end())
				command.modeCommand(client, *(it + 1), *(it + 2), *(it + 3));
			else
				command.modeCommand(client, *(it + 1), *(it + 2), "NULL");
			return;
		}

		if(equalStrings(*it, "INVITE") && client->isEligible())
		{

			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)) 
				|| emptyParam(vec, (it + 2), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
			{
				return ;
			}

			if (getClientFromServer(*(it + 1)) != NULL)
			{
				Client *invitedClient = getClientFromServer(*(it + 1));
				if (!command.channelExist(*(it + 2)))
				{
					const std::string &msg = ":IRC " + ERR_NOSUCHCHANNEL(client->getNickName(), *(it + 2));
					send(client->getClientSock(), msg.c_str(), msg.size(), 0);
					return ;
				}
				if(command.getChannelByName(*(it + 2))->checkClientIsModerator(client->getClientSock()) == false)
				{
					const std::string &msg = ":IRC " + ERR_CHANOPRIVSNEEDED(client->getNickName(), *(it + 1));
					send(client->getClientSock(), msg.c_str(), msg.size(), 0);
					return ;
				}
				int check;
				check = command.inviteclientcheck(invitedClient, *(it + 2));
				if(check == 3)
				{
					const std::string &msg = ":IRC " + ERR_USERONCHANNEL(client->getNickName(), *(it + 2), *(it + 1));
					send(client->getClientSock(), msg.c_str(), msg.size(), 0);
					return;
				}

				command.getChannelByName(*(it + 2))->AddUser2Channel(invitedClient);
				const std::string& msg = command.standardMsg(client->getNickName(), client->getUserName(), client->getIpAddress()) + ".IP INVITE " + invitedClient->getNickName() + " " + command.getChannelByName(*(it + 2))->getChannelName() + "\r\n";
				command.sendData(invitedClient->getClientSock(), msg);


				const std::string& name = command.getChannelByName(*(it + 2))->getChannelName();
				const std::string& ms = command.standardMsg(invitedClient->getNickName(), invitedClient->getUserName(), invitedClient->getIpAddress()) + " JOIN " + name + " * " + invitedClient->getRealName() + " " + RPL_WELCOME(invitedClient->getNickName(), "IRC");
				command.getChannelByName(*(it + 2))->sendToAll(ms);
				return;
			}
			else
			{
				const std::string &msg = ":IRC " + ERR_NOSUCHNICK(client->getNickName(), *(it + 1));
				send(client->getClientSock(), msg.c_str(), msg.size(), 0);
				return;
			}
			return;

		}
		if(equalStrings(*it, "KICK") )
		{
			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)) 
				|| emptyParam(vec, (it + 2), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
				return ;
			if(*(it + 2)->begin() == '#')
			{
				const std::string  &msg = RPL_KICKED(client->getUserName(), *(it + 1), client->getNickName());
				
				command.sendData(client->getClientSock(), msg);
				return;
			}
			Client *kickedClient = getClientFromServer(*(it + 2));
			if(!kickedClient)
			{
				const std::string  &msg = ":IRC " + ERR_USERNOTINCHANNEL(client->getNickName(), *(it + 1), *(it + 2))  ;
				command.sendData(client->getClientSock(), msg);
				return ;
			}
			std::string m = " ";
			if(it + 3 != vec.end())
				m = getRangeAsString(vec, it + 3, vec.size(), " ");
			else
				m = "NO REASON INCLUDED... ";
			command.kickCommand(client, *(it + 1), *(it + 2), kickedClient, m);
			return;
		}
		if(equalStrings(*it, "PART") )
		{
			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
				return ;
			std::string m = " ";
			if(it + 2 != vec.end())
				m = getRangeAsString(vec, it + 2, vec.size(), " ");
			else
				m = "NO REASON INCLUDED... ";
			command.partCommand(client, *(it + 1), m);

		}
		if(equalStrings(*it, "WHO") )
		{
			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
				return ;
			Channel *channel = command.getChannelByName(*(it + 1));
			if(!channel)
			{
				const std::string &msg = ":IRC " + ERR_NOSUCHCHANNEL(client->getNickName(), *(it + 1));
				command.sendData(client->getClientSock(), msg.c_str());
				return ;
			}
			std::vector<Client>* users = channel->getChannelClientsVector();
			command.rpl_list(client, channel); 
            for (std::vector<Client>::iterator it = users->begin(); it != users->end(); ++it)
            {
                Client user = *it;
                std::string msg;
                if (channel->checkClientIsModerator(user.getClientSock()))
                    msg = ":" + this->serverName + RPL_WHOREPLY(client->getNickName(), channel->getChannelName(), user.getUserName(), user.getIpAddress(), "0.Matrix ", user.getNickName(), "@x", user.getRealName());
                else
                    msg = ":" + this->serverName + RPL_WHOREPLY(client->getNickName(), channel->getChannelName(), user.getUserName(), user.getIpAddress(), "0.Matrix ", user.getNickName(), "x", user.getRealName());
                send(client->getClientSock(), msg.c_str(), msg.size(), 0);
            }
            std::string msg = ":" + this->serverName + RPL_ENDOFWHO(client->getNickName(),  channel->getChannelName());
            send(client->getClientSock(), msg.c_str(), msg.size(), 0);
		}
		if(equalStrings(*it, "BOT") && client->isEligible())
		{
			if(it + 1 == vec.end())
			{
				std::string msg = "Choose Between : FACTS( Enter 1 ) , AGE( Enter 2 ), INFOS( Enter 3 ).\r\n";
				command.sendData(client->getClientSock(), msg);
				return;
			}




			if(it + 1 != vec.end())
			{
				if(equalStrings(*(it + 1), "1"))
					facts(client->getClientSock());
				else if(equalStrings(*(it + 1), "2"))
				{
					std::string msg = "LIST OF CHANNELS IN THE SERVER :\n";
					std::vector<Channel>::iterator chan = command.getChannelVector()->begin();
					while (chan != command.getChannelVector()->end())
					{
						msg += chan->getChannelName() + "\n";
						chan++;
					}
					command.sendData(client->getClientSock(), msg);
					std::string msg1 = "LIST OF CLIENTS IN THE SERVER :\n";
					std::vector<Client>::iterator it = clients.begin();
					while (it != clients.end())
					{
						msg1 += it->getNickName() + "\n";
						it++;
					}
					command.sendData(client->getClientSock(), msg1);
					return;
					
				}
				else if(equalStrings(*(it + 1), "3"))
				{
					if(it + 2 == vec.end() || it + 3 == vec.end() || it + 4 == vec.end())
						command.sendData(client->getClientSock(), "PLEASE CORRECT SYNTAX (BOT 3 (day) (month) (year))");
					else
						ageCalculator(*(it + 2), *(it + 3), *(it + 4), client->getClientSock());
				}
				else 
				std::string msg = "Incorrect choice please Choose Between : FACTS( Enter 1 ) , AGE( Enter 2 ), INFOS( Enter 3 ).\r\n";
				command.sendData(client->getClientSock(), msg);
				return;

			}
			return;
		}
		if(equalStrings(*it, "TOPIC") )
		{
			if (emptyParam(vec, (it + 1), client->getClientSock(), ERR_NEEDMOREPARAMS(client->getNickName(), *it)))
			{
				return;
			}
			Channel *channel = command.getChannelByName(*(it + 1));
			if(!channel)
			{
				const std::string &msg = ":IRC " + ERR_NOSUCHCHANNEL(client->getNickName(), *(it + 1));
				std::cout << "1\n";
				command.sendData(client->getClientSock(), msg.c_str());
				return ;
			}
			if(command.inviteclientcheck(client, channel->getChannelName()) != 3)
			{
				const std::string &msg = ":IRC " + ERR_USERNOTINCHANNEL(client->getNickName(), client->getNickName(), channel->getChannelName());
				send(client->getClientSock(), msg.c_str(), msg.size(), 0);
				return;
			}
			if((it + 2) == vec.end())
			{
				std::string topic = channel->getTopic();
				const std::string &msg = ":IRC " + RPL_TOPIC(client->getNickName(), *(it + 1),topic);
				int r = send(client->getClientSock(), msg.c_str(), msg.size(), 0);
				std::cout << r << std::endl;
				return;
			}
			else if ((it+2) != vec.end())
			{
				if(command.getChannelByName(*(it + 1))->getTopicMode() == true)
				{
					if(command.getChannelByName(*(it + 1))->checkClientIsModerator(client->getClientSock()))
					{
						const std::string &msg = ":IRC " + ERR_CHANOPRIVSNEEDED(client->getNickName(), *(it + 1));
						send(client->getClientSock(), msg.c_str(), msg.size(), 0);
						return ;
					}
				}
					std::cout << "before EQual :" << (it + 2)->size() << std::endl;
					if(equalStrings(*(it + 2), "::"))
					{
						(it + 2)->size();
						std::cout << "555\n";
						channel->setTopic("No topic is set");
						return;
					}
				std::cout << "6\n";
				std::string range = getRangeAsString(vec, it + 2, vec.size(), " ");
				channel->setTopic(range);
			}
			std::vector<Client>* vec = channel->getChannelClientsVector();
			std::vector<Client>::iterator ite = vec->begin();
			while(ite != vec->end())
			{
				std::string topic = channel->getTopic();
				const std::string &msg = ":IRC " + RPL_TOPIC(ite->getNickName(), *(it + 1),topic);
				 send(ite->getClientSock(), msg.c_str(), msg.size(), 0);
				ite++;
			}
		}
		if (*it == "QUIT" && *(it + 1) == ":Leaving")
		{

			command.removeClientFromAllChannels(client->getClientSock());
			removeClientFromPollfd(client->getClientSock());
			return;
		}
		it++;
	}
}