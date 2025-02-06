#include "../includes/Channel.hpp"
#include <sstream>
#include <unistd.h>


Channel::Channel() : requirePasswd(false), inviteMode(0), topicMode(0), channelLimit(100) ,channelName("")
{

}


Channel::Channel(const std::string& _channelName) : requirePasswd(false), inviteMode(0), topicMode(0), hasLimit(0), channelLimit(100) ,channelName(_channelName), channelPasswd(""), channelClients(0), topic(" No topic is set")
{

}


Channel::~Channel()
{
}

void Channel::AddUser2Channel(Client* client)
{
	this->channelClients.push_back(*client);
}


std::string Channel::getChannelName() const
{
	return this->channelName;
}

void Channel::setInviteMode(const bool &mode)
{
	this->inviteMode = mode;
}


bool Channel::getInviteMode() const
{
	return this->inviteMode;
}

std::vector<Client> *Channel::getChannelClientsVector()
{
	return &(channelClients);
}

std::string Channel::getChannelClientByName()
{
	std::string names;
	size_t i = 0;
	while (i < channelClients.size())
	{
		names += channelClients[i].getNickName();
		i++;
		if (i < channelClients.size())
			names += "\n";
	}
	return names;
}

int Channel::getCurrentTimestamp() 
{
    return static_cast<int>(std::time(nullptr)); 
}


bool Channel::channelIsFull()
{
	return (channelClients.size() == this->channelLimit ? true : false);
}

bool Channel::userExistInChannelByName(const std::string &name, const int& nb)
{
	std::vector<Client>::iterator it = channelClients.begin();

	while (it != channelClients.end())
	{
		if (it->getClientSock() != nb)
		{
			if (it->getNickName() == name)
				return true;
		}
		it++;
	}
	return false;
}

bool Channel::userExistInChannelBySock(const int& nb)
{
	std::vector<Client>::iterator it = channelClients.begin();

	while (it != channelClients.end())
	{
		if (it->getClientSock() == nb)
			return true;
		it++;
	}
	return false;
}

std::string Channel::getPasswd() const
{
	return this->channelPasswd;
}


void Channel::setPasswdRequired(const bool& val)
{
	this->requirePasswd = val;
}

void Channel::setPasswd(const std::string& passwd)
{
	this->channelPasswd = passwd;
	if (!passwd.empty())
		setPasswdRequired(true);
	return;
}

bool Channel::getPasswdRequired() const
{
	return this->requirePasswd;
}


bool Channel::channelInviteModeOnly()
{
	return (this->inviteMode == true ? true : false);
}


size_t Channel::getChannelClientSize()
{
	return this->channelClients.size();
}

void Channel::resizeClientLimit(const size_t& i)
{
	if (i >= channelClients.size())
		this->channelLimit = i;
	else
		std::cout << "The number should be greater or equal to the number of clients in " << getChannelName() << std::endl;
}

Client *Channel::getClientFromChannelByName(const std::string& name)
{
	size_t i = 0;
	while (i < channelClients.size())
	{
		if (channelClients[i].getNickName() == name)
			return &(channelClients[i]);
		i++;	
	}
	return NULL;
}

void Channel::removeClientFromChannel(const Client& client)
{
	std::vector<Client>::iterator it = channelClients.begin();
	while (it != channelClients.end())
	{
		if (it->getNickName() == client.getNickName())
		{
			channelClients.erase(it);
			return;
		}
		it++;
	}
}



void Channel::setChannelLimit(const size_t& limit)
{
	this->channelLimit = limit;
}


size_t Channel::getChannelLimit() const
{
	return this->channelLimit;
}

void Channel::setTopicMode(const bool& val)
{
	this->topicMode = val;
}
bool Channel::getTopicMode() const
{
	return this->topicMode;
}

std::string Channel::getTopic() const
{
	return this->topic;
}

void Channel::setTopic(const std::string & _topic)
{
	this->topic = _topic;
}

void Channel::setHasLimit(const size_t& val)
{
	this->hasLimit = val;
}
bool Channel::getHasLimit() const
{
	return this->hasLimit;
}

std::string Channel::getChannelMode() const
{
	std::string modes = "";
	if (this->getInviteMode())
		modes += " +i";
	if (this->getTopicMode())
		modes += " +t";
	if (this->getPasswdRequired())
		modes += " +k";
	if (this->getHasLimit())
		modes += " +l";
	if (this->operators.size() > 1)
		modes += " +o";
	return modes;
}

void Channel::sendToAll(const std::string& msg)
{
    std::vector<Client>::iterator it = channelClients.begin();
    while(it != channelClients.end())
    {
        send(it->getClientSock(), msg.c_str(), msg.size(), 0);
        it++;
    }
}


void Channel::addModerator(const Client *client)
{
	this->operators.push_back(*client);
}

void Channel::removeModerator(const Client *client)
{
	std::vector<Client>::iterator it = operators.begin();
    while(it != operators.end())
    {
        if (it->getClientSock() == client->getClientSock())
		{
			operators.erase(it);
			return ;
		}
        it++;
    }
}


bool Channel::checkClientIsModerator(const int& fd)
{
	std::vector<Client>::iterator it = operators.begin();
	while (it != operators.end())
	{
	
		if (it->getClientSock() == fd)
			return true;
		it++;
	}
	return false;
}

void Channel::printOp()
{
	std::vector<Client>::iterator it = operators.begin();
	while (it != operators.end())
	{
	
		std::cout << it->getNickName() << "\n";
		it++;
	}
}