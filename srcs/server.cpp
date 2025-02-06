#include "../includes/Server.hpp"

Server::Server() : serverSock(-1), passwd(""), serverName("IRC ")
{
	this->monitor.clear();
	this->clients.clear();
}

Server::~Server()
{
}

/***********************************/
/*                                 */
/* FUNCTIONS SETTING UP THE SERVER */
/*                                 */
/***********************************/

/* Creating the server and making it ready to recieve the incoming connections */
void Server::setServerSock(int port)
{
	struct pollfd serverPoll;
	int optval = 1;
	serverAddr.sin_family = AF_INET; // for IPv4 usage
    serverAddr.sin_addr.s_addr = INADDR_ANY; // address to communicate with
    serverAddr.sin_port = htons((uint16_t)port); //port to communicate from

	this->serverSock = socket(AF_INET, SOCK_STREAM, 0);

	if(this->serverSock < 3)
		throwError("Socket Failed", 0);

	int sockopt = setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (sockopt < 0)
		std::cout << "chi l3ayba tema " << std::endl;
	if (fcntl(serverSock, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));

	int _bind = bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (_bind == -1)
		throwError("Bind failed => ", serverSock);

	if (listen(serverSock, SOMAXCONN) == -1)
        throwError("Listen failed", serverSock);

	serverPoll.fd = serverSock;
	serverPoll.events = POLLIN;
	this->monitor.push_back(serverPoll);
}

void Server::facts(const int &cs)
{
	std::srand(std::time(NULL));
	int choice = (rand() % 20) + 1;
	if(choice == 1)
		command.sendData(cs, "Bananas are berries, but strawberries aren't.\n");
	else if(choice == 2)
		command.sendData(cs, "Octopuses have three hearts.\n");
	else if(choice == 3)
		command.sendData(cs, "Wombat poop is cube-shaped.\n");
	else if(choice == 4)
		command.sendData(cs, "Sharks existed before trees.\n");
	else if(choice == 5)
		command.sendData(cs, "The Eiffel Tower can grow taller in summer.\n");
	else if(choice == 6)
		command.sendData(cs, "Honey never spoils.\n");
	else if(choice == 7)
		command.sendData(cs, "Water can boil and freeze at the same time.\n");
	else if(choice == 8)
		command.sendData(cs, "A day on Venus is longer than a year on Venus.\n");
	else if(choice == 9)
		command.sendData(cs, "The human body glows in the dark.\n");
	else if(choice == 10)
		command.sendData(cs, "There's a species of jellyfish that can live forever.\n");
	else if(choice == 11)
		command.sendData(cs, "You can hear a blue whaleâ€™s heartbeat from 2 miles away.\n");
	else if(choice == 12)
		command.sendData(cs, "There's a gas cloud in space that smells like rum and tastes like raspberries.\n");
	else if(choice == 13)
		command.sendData(cs, "Butterflies can taste with their feet.\n");
	else if(choice == 14)
		command.sendData(cs, "Some turtles can breathe through their butts.\n");
	else if(choice == 15)
		command.sendData(cs, "The entire worldâ€™s population could fit in Los Angeles.\n");
	else if(choice == 16)
		command.sendData(cs, "There's an island within a lake, on an island, in a lake, on an island.\n");
	else if(choice == 17)
		command.sendData(cs, "Sloths can hold their breath longer than dolphins.\n");
	else if(choice == 18)
		command.sendData(cs, "Scotland has 421 words for snow.\n");
	else if(choice == 19)
		command.sendData(cs, "Earth once had two moons.\n");
	else if(choice == 20)
		command.sendData(cs, "The shortest war in history lasted only 38 minutes.\n");
	else 
		command.sendData(cs, "Invalid choice! Please enter a number between 1 and 20.\n");
	return ;
}

/* Accepting the incoming connection from the clients */
void Server::acceptNewConnection()
{
	Client client;
	int newsocket;
	
	struct sockaddr_in sockAddrClient;  // The information about the client (IPv4/v6, port) will be filled in this struct
	struct pollfd clientPoll;
	socklen_t len = sizeof(sockAddrClient);

 	newsocket = accept(serverSock, (struct sockaddr*)&sockAddrClient, &len);
	if (newsocket < 0) {
        throwError("Conenction failed", serverSock);
	}
	std::cout << BCYAN << "Connection is established successfuly with client N˚" << newsocket - 3 << ENDC << std::endl;


	clientPoll.fd = newsocket;
	clientPoll.events = POLLIN; 

	client.setIpAddress(inet_ntoa(sockAddrClient.sin_addr));
	client.setClientSock(newsocket);
	this->clients.push_back(client);
	this->monitor.push_back(clientPoll);
}



void Server::runningServer(int port, const char *av)
{
	this->setServerSock(port);
	this->setServerPassWd(av);

	while(Server::running == true)
	{


		if (poll(&(monitor[0]), monitor.size(), 0) == -1)
			this->throwError("poll failed", 0);

		for (size_t i = 0; i < monitor.size(); i++)
		{
			if (monitor[i].revents & POLLIN)
			{
				
				if (monitor[i].fd == getServerFd())
					acceptNewConnection();
				else
					recieveData(monitor[i].fd);
			}
		}
	}
	
	this->closeFd();
}



/************************************************/
/*          									*/
/* FUCNTIONS HANDLING SENDING AND RECIEVING MSG */
/*                          			        */
/************************************************/


/* Reading msg from client */
void Server::recieveData(int clientSock)
{
	char message[1024];
	int rbyte = recv(clientSock, message, sizeof(message) - 1, 0);
	if (rbyte <= 0)
	{
		command.removeClientFromAllChannels(clientSock);
		close(clientSock);
		std::cout << "Client {" << clientSock - 3 << "}" << " has been Disconnected." << std::endl;
		return;
	}
	message[rbyte] = '\0';
	Client *client = this->getClientFromVectorByFd(clientSock); // Pointing to the client who sent the message in the vector
		processCommand(client, message);

}

/* Sending msg to client */

/*********/
/*       */
/* UTILS */
/*       */
/*********/

void Server::throwError(const char* msg, int fd)
{
	close(fd);
	perror(msg);
	_exit(0);
}

void Server::closeFd()
{
	for(size_t i = 0; i < monitor.size(); i++)
		close(monitor[i].fd);
}


bool Server::running = true;

void Server::signalHandler(int sig)
{
	(void)sig;
    std::cout << std::endl << "Received SIGUSR1 signal!" << std::endl;
    Server::running = false;
	exit(1);}


void Server::removeClientFromServer(int clientSock)
{
	std::vector<Client>::iterator it = clients.begin();
	while (it != clients.end())
	{
		if (it->getClientSock() == clientSock)
			clients.erase(it);
		it++;
	}
}



void Server::removeClientFromPollfd(int clientsock)
{
	std::vector<struct pollfd>::iterator it = monitor.begin();
	for (it = monitor.begin(); it < monitor.end(); it++)
	{
		const int &i = it->fd;
		if (i == clientsock)
		{
			close(it->fd);
			monitor.erase(it);
		}
	}
	return;
}

/***********/
/*         */
/* SETTERS */
/*         */
/***********/

void Server::setServerPassWd(const char* av)
{
	this->passwd = av;
}


int Server::getServerFd() const
{
	return this->serverSock;
}

/***********/
/*         */
/* GETTERS */
/*         */
/***********/

std::vector<struct pollfd> Server::getMonitor() const
{
	return this->monitor;
}


uint16_t Server::getMonitorSize() const
{
	return this->monitor.size();
}



Client* Server::getClientFromVectorByFd(int _clientSock)
{
	Client* p = NULL;
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->getClientSock() == _clientSock)
		{
			p = &(*it);
			return p;
		}
	}
	return NULL;
}


std::string Server::getPasswd() const
{
	return this->passwd;
}

std::string Server::getName() const
{
	return this->serverName;
}



std::vector<std::string> Server::getWords_(const std::string &str)
{

	std::vector<std::string> words;
	std::istringstream stream(str);
	std::string word;

	// Extract words using the stream extraction operator
	while (stream >> word)
	{
		words.push_back(word);
	}
	return words;
}


std::string Server::getRangeAsString(const std::vector<std::string>& vec, std::vector<std::string>::iterator it,  size_t end, std::string delimiter) 
{
	if (it >= vec.end() || end > vec.size()) 
		return "";
	// Extract range and concatenate strings
	std::string result = "";
	while (it != vec.begin() + end) 
	{
		if (!result.empty()) 
			result += delimiter; // Add a space between words
		result += *it;
		it++;
	}
	return result;
}

Client *Server::getClientFromServer(const std::string& nickname)
{
	size_t i = 0;
	while(i != clients.size())
	{
		if (clients[i].getNickName() == nickname)
			return &(clients[i]);
		i++;
	}
	return NULL;
}


Client *Server::getServerClient(const std::string &str)
{
	size_t i = 0;
	while (i != this->clients.size())
	{
		if (clients[i].getNickName() == str)
			return &(clients[i]);
		i++;	
	}
	return NULL;
}

// true when strings are identical
bool Server::equalStrings(const std::string& it, const std::string& compare)
{
	return (strncmp(it.c_str(), compare.c_str(), compare.size()) == 0);
}


void Server::notCommand(const std::string& str)
{
	if (str != "PASS"
		&& str != "INVITE"
		&& str != "MODE"
		&& str != "NICK"
		&& str != "USER"
		&& str != "KICK"
		&& str != "PRIVMSG"
		&& str != "PART"
		&& str != "WHO")
		return;
}

bool Server::emptyParam(const std::vector<std::string>& vec, const std::vector<std::string>::iterator& param, const int &fd, const std::string &msg)
{
	
	if (param == vec.end())
	{
		send(fd, msg.c_str(), msg.size(), 0);
		return true;
	}
	return false;
}

bool Server::nickNameInUse(Client *client, const std::string& name)
{
	std::vector<Client>::iterator it = clients.begin();

	while (it != clients.end())
	{
		if (it->getClientSock() != client->getClientSock())
		{
			if (strncmp(it->getNickName().c_str(), name.c_str(), it->getNickName().size()) == 0)
				return true;
		}
		it++;
	}
	return false;
}


void Server::ageCalculator(std::string d, std::string m, std::string y, const int &cs)
{
	int day = std::atoi(d.c_str());
	int month = std::atoi(m.c_str());
	int year = std::atoi(y.c_str());
    // std::cout << "Enter your birthdate (day month year): ";
    // std::cin >> day >> month >> year;
	if(year > 2025 || year < 0 || month < 1 || month > 12 || day < 1 || day > 31)
	{
		command.sendData(cs, "error");
		return ;
	}
	else 
	{
		if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
		{
			if(day > 31 ||  day < 1)
			{
				command.sendData(cs, "error");
				return ;
			}
		}
		else if(month == 4 || month == 6 || month == 9 || month == 11)
		{
			if(day > 30 || day < 1)
			{
				command.sendData(cs, "error");
				return ;
			}
		}
		else
		{
			if(year % 4 == 0)
			{
				if(month > 29)
				{
					command.sendData(cs, "error");
					return ;
				}
			}
			else 
			{
				if(month > 28)
				{
					command.sendData(cs, "error");
					return ;
				}
			}
		} 
	}
    // Get the current time
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);
    // std::tm* now_tm = std::localtime(&current_time);

    // Set the birthday date (assuming 00:01 AM of the given day)
    std::tm birth_tm = {};
    birth_tm.tm_year = year - 1900; // tm_year is years since 1900
    birth_tm.tm_mon = month - 1;    // tm_mon is months since January (0-11)
    birth_tm.tm_mday = day;
    birth_tm.tm_hour = 0;           // Midnight time
    birth_tm.tm_min = 1;            // 00:01 AM

    // Convert to time_t (seconds since epoch)
    std::time_t birth_time = std::mktime(&birth_tm);

    // Calculate the difference in seconds
    double seconds = std::difftime(current_time, birth_time);

    // Calculate age in different units
    long long age_seconds = static_cast<long long>(seconds);
    long long age_minutes = age_seconds / 60;
    long long age_hours = age_minutes / 60;
    long long age_days = age_hours / 24;
    long long age_weeks = age_days / 7;
    long long age_months = age_days / 30;  // Approximating 30 days per month

 std::string result = "Your age is:\n";
    result += "In seconds: " + std::to_string(age_seconds) + " secs\n";
    result += "In minutes: " + std::to_string(age_minutes) + " mins\n";
    result += "In hours: " + std::to_string(age_hours) + " hours\n";
    result += "In days: " + std::to_string(age_days) + " days\n";
    result += "In weeks: " + std::to_string(age_weeks) + " weeks\n";
    result += "In months: " + std::to_string(age_months) + " months\n";
    // Print the results
	command.sendData(cs,  result);
}

