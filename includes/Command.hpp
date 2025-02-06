#pragma once
#define CRLF "\r\n"

#define ERR_FUNCSEND "FATAL : send() Failed!"
#include <iostream>
#include <cstring> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <poll.h>
#include <sstream>
#include <unistd.h>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "Replies.hpp"


class Command
{
public :
	Command();
	~Command();

	
	bool channelExist(const std::string &);
	bool invalidNickName(const std::string&);


	void	eligibiltyErr(Client *, const std::string&); //processing the returned value of isEligible() function
	void 	joinCommand(Client *, const std::string &, const std::string&);
	void 	modeCommand(Client *,const std::string &, const std::string &, const std::string &);
	void 	passCommand(Client *, const std::string &, const std::string &);
	void 	privmsgCommandChannel(const std::string &, Client *, const std::string &);
	void 	privmsgCommandUser(Client *, Client *, const std::string &);
	void	removeClientFromAllChannels(const int& toremove);
	void 	sendData(int, const std::string &);
	void	sendToAll(Client *client,const std::string msg, std::string name);

	int	 inviteclientcheck(Client* client, std::string const &name);
	void kickCommand(Client *client,  const std::string& channelName, const std::string& clientName, Client *clientToKick, std::string m);
	int  clientinthechannel(std::string const &chaine, std::string const &name);
	int	 userinthechannel(Client* client, std::string const &name, std::string const &usname);
	int kickClientFromChannel(const std::string &chaine,Client *client);
	void inputClient(Client *client, std:: string name);
	void partCommand(Client *client,const std::string &channelName, const std::string &message);
	void rpl_list(Client *client, Channel *channel);
	void removeChannelFromVector(const std::string& channame);



	std::string					standardMsg(std::string, std::string, std::string);
	std::vector<Channel>		*getChannelVector() ;
	std::vector<std::string>	getWords(const std::string &);


	Channel	*getChannelByName(const std::string &);
private :
	std::string commandLine;
	std::string parameter1;
	std::string parameter2;
	std::vector<Channel> channels;
};


