#include "Server.hpp"

Server::Server(int _port, std::string _password) : port(_port), password(_password)
{
	
}

void	Server::start_server(void)
{
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        throw std::runtime_error("Socket creation failed");

    pollfd server_pollfd;
    server_pollfd.fd = _socket;
    server_pollfd.events = POLLIN;
    server_pollfd.revents = 0;
    fds.push_back(server_pollfd);

	memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;  

    if (bind(_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        close(_socket);
        throw std::runtime_error("Binding failed");
    }

    if (listen(_socket, 5) == -1) 
    {
        close(_socket);
        throw std::runtime_error("Listening failed");
    }

    std::cout << "Server is listening on port " << port << " with password " << password << "...\n";
}

void	Server::remove_client(std::map<int, User *>::iterator it, int x)
{
    for (std::map<std::string, Channel *>::iterator chan = channel_list.begin(); chan != channel_list.end(); chan++)
    {
        if (it->second->is_joined(chan->first))
            chan->second->remove_from_list(it->second);
    }
    std::cout << "Client " << it->second->get_nickname() << " requested to close the connection." << std::endl;
    unavailable_nick.erase(it->second->get_nickname());
    std::vector <struct pollfd>::iterator it2 = fds.begin();
    std::advance(it2, x);
    fds.erase(it2);
    client_list.erase(it);
}

void	send_msg(int client_fd, std::string message)
{
    int temp = 0;

    message.append("\r\n");
    for (size_t bytes_sent = 0; bytes_sent < message.length();)
    {
        if ((temp = send(client_fd, message.c_str(), message.length(), 0) ) == -1)
        {
            std::cerr << "send() error" << std::endl;
            return ;
        }
        else
            bytes_sent += temp;
    }
}

int is_auth_character(const std::string& str)
{

    const char spe_character[] = {'`', '|', '^', '_', '-', '{', '}', '[', ']', '\\'};
    std::set<char> spe_character_set(spe_character, spe_character + sizeof(spe_character) / sizeof(char));

    for (size_t x = 0; x < str.length(); x++)
    {
        if (!isalnum(str[x]) && spe_character_set.find(str[x]) == spe_character_set.end())
            return 0;
    }

    return 1;
}

int	Server::get_socket() const
{
	return (_socket);

}

struct sockaddr_in	Server::get_server_adress() const
{
	return (server_address);
	
}

std::string	Server::get_password() const
{
	return (password);

}

std::map<int, User*>	Server::get_client_list() const
{
	return (client_list);

}

std::map<std::string, Channel *>	Server::get_channel_list() const
{
	return (channel_list);

}

std::vector<struct pollfd>	Server::get_fds() const
{
	return (fds);

}

std::set<std::string>	Server::get_unavilable_nick() const
{
	return (unavailable_nick);
}

Server::~Server()
{

    for (std::map<int, User *>::iterator it = client_list.begin(); it != client_list.end(); ++it)
        delete it->second;
    for (std::map<std::string, Channel *>::iterator it = channel_list.begin(); it != channel_list.end(); ++it)
        delete it->second;

    if (_socket != -1)
        close(_socket);

    client_list.clear();
    channel_list.clear();
    fds.clear();
    unavailable_nick.clear();
}

/*-------------Nucleocherry's functions-------------*/

int					Server::get_client_fd_by_nickname(std::string _nickname)
{
	for (std::map<int, User *>::iterator it = client_list.begin(); it != client_list.end(); it++)
	{
		if (it->second->get_nickname() == _nickname)
		{
			return (it->second->get_client_fd());
		}
	}
	return (-1);
}

void                Server::kick_user(std::string demand, int client_fd)
{
    std::vector<std::string> buffer = parse_request((char*)(demand.c_str()), " :*\r\n", 4);
    
    buffer.erase(buffer.begin());

    if (buffer.size() < 2)
    {
        send_msg(client_list[client_fd]->get_client_fd(), "IRC ERR_NEEDMOREPARAMS");
        return ;
    }


    int kickedClient = get_client_fd_by_nickname(buffer[1]);
    if (kickedClient == -1/*cherche si il existe*/)
    {
        send_msg(client_list[client_fd]->get_client_fd(), "IRC 441 ERR_USERNOTINCHANNEL");
        return ;
    }
    std::string channelName = buffer[0];
    if (channel_list[channelName]->get_operator() == client_list[client_fd]->get_client_fd())
    {
        if (client_list[client_fd]->get_client_fd() == kickedClient)
        {
            send_msg(client_list[client_fd]->get_client_fd(), "IRC Op cant kick himself");
            ;
            return ;
        }
        std::string msg = ":" + client_list[client_fd]->get_nickname() + "!" + client_list[client_fd]->get_username() + "@host KICK " + channelName + " " + client_list[kickedClient]->get_nickname() + " :";
        if (buffer.size() == 3)
        {
            std::string com = buffer[2];
            com = remove_endl(com, '\n');
            com = remove_endl(com,'*');
            com = remove_endl(com, ':');
            msg += com;
        }
        std::cout << "msg: " << msg << std::endl;
        channel_list[channelName]->send_to_all(-1, msg);
        channel_list[channelName]->remove_from_list(client_list[kickedClient]);
        send_msg(client_fd, "IRC Kicked worked successfully");
    }
    else
        send_msg(client_fd, "IRC 482 ERR_USERNOTOPERATOR");
    ;
}

void                Server::invite_user(int client_fd, std::string demand)
{
    std::vector<std::string> buffer = parse_request((char*)(demand.c_str()), " :*\r\n", 3);
    buffer.erase(buffer.begin());

    if (buffer.size() < 2)
    {
        send_msg(client_fd, "IRC ERR_NEEDMOREPARAMS");
        return ;
    }

    int invitedClient = get_client_fd_by_nickname(buffer[0]);
    std::string channelInvited_name(buffer[1]);

    if (invitedClient == -1)
    {
        send_msg(client_fd, "IRC ERR_USER_DOESNOTEXIST");
        return ;
    }

    if (!channel_list[channelInvited_name])
    {
        send_msg(client_fd, "IRC ERR_CHANNEL_DOESNOTEXIST");
        return ;
    }
    if (channel_list[channelInvited_name]->get_user_limit() == (int)channel_list[channelInvited_name]->get_client_list().size())
    {
        send_msg(client_fd, "IRC ERR_CHANNEL_FULL");
        return ;
    }
    if (client_list[invitedClient]->is_joined(channelInvited_name)/*cherche si il existe*/)
    {
        send_msg(client_fd, "IRC ERR_USER_ALREADY_IN");
        return ;
    }

    std::string msg = ":" + client_list[client_fd]->get_nickname() + "!" + client_list[client_fd]->get_username() + "@host INVITE " + client_list[invitedClient]->get_nickname() + ":" + channelInvited_name;
    std::string msg2 = "IRC_name 341 " + client_list[client_fd]->get_nickname() + " " + client_list[invitedClient]->get_nickname() + " " + channelInvited_name;
    send_msg(invitedClient, msg);
    send_msg(client_fd, msg2);
   client_list[invitedClient]->set_invitedChannel(channel_list[channelInvited_name]);
}

void	Server::change_topic(int client_fd, std::string demand)
{
	std::vector<std::string> buffer = parse_request((char*)(demand.c_str()), " :*\r\n", 3);
    
    buffer.erase(buffer.begin());

    if (buffer.size() < 1)
    {
        send_msg(client_fd, "IRC ERR_NEEDMOREPARAMS");
        return ;
    }
	if (buffer.size() == 1)
	{
		std::string msg = "IRC TOPIC : " +  channel_list[buffer[1]]->get_topic();
		send_msg(client_fd, msg);
        ;
		return ;
	}

	std::string channelName = buffer[0];
	if (channel_list[channelName]->get_operator() != client_fd && channel_list[channelName]->get_topic_changement() == false)
	{
		send_msg(client_fd, "IRC You cannot change the topic");
        ;
		return ;
	}
	std::cout << "len of buffer : " << buffer.size() << std::endl;
	if (buffer.size() >= 2)
	{
		channel_list[buffer[0]]->set_topic(buffer[1]);
		send_msg(client_fd, "IRC Topic changed successfully");
	}
    ;
}

/*void	Server::help(int client_fd, std::string demand)
{
	std::vector<std::string> buffer = parse_request((char*)(demand.c_str()), " :*\r\n", 2);

    if (buffer.size() == 1)
    {
        send_msg(client_fd, "IRC [KICK] [INVITE] [TOPIC] [MODE] [PART] [NICK] [JOIN] [USER] [PASS] [PRIVMSG] [HELP] [CAP LS 302] [QUIT] [CAP END]");
        return ;
    }
    else if (strncmp(buffer[1], "KICK", 4) == 0)
		send_msg(client_fd, "IRC (operator only) /KICK <channel> <user> [:<reason>]\n Used to remove an user from a channel");
    else if (strncmp(buffer[1], "INVITE", 6) == 0)
		send_msg(client_fd, "IRC /INVITE <nickname> <channel>\n Used to invite an user to a private channel");
    else if (strncmp(buffer[1], "TOPIC", 5) == 0)
		send_msg(client_fd, "IRC (operator only) /TOPIC <channel> [:<new topic>]\n Used to change the topic of a channel");
    else if (strncmp(buffer[1], "MODE", 4) == 0)
		send_msg(client_fd, "IRC (operator only) /MODE <channel|nickname> [<mode>] [parameters]\n Used to setup the channel's mode");
    else if (strncmp(buffer[1], "PART", 4) == 0)
        send_msg(client_fd, "IRC /PART <channel> [:<message>]\n Used to leave a channel optionally with a message");
    else if (strncmp(buffer[1], "NICK", 4) == 0)
        send_msg(client_fd, "IRC /NICK <new nickname>\n Used to change your nickname");
    else if (strncmp(buffer[1], "JOIN", 4) == 0)
        send_msg(client_fd, "IRC /JOIN <channel> [<key>]\n Used to join a channel, optionally with a password");
    else if (strncmp(buffer[1], "USER", 4) == 0)
        send_msg(client_fd, "IRC /USER <username> <hostname> <servername> :<realname>\n Used to register a new user with the server");
    else if (strncmp(buffer[1], "PASS", 4) == 0)
        send_msg(client_fd, "IRC /PASS <password>\n Used to authenticate with the server using a password");
    else if (strncmp(buffer[1], "PRIVMSG", 7) == 0)
        send_msg(client_fd, "IRC /PRIVMSG <receiver> :<message>\n Used to send a private message to a user or a channel");
    else if (strncmp(buffer[1], "CAP LS 302", 10) == 0)
        send_msg(client_fd, "IRC /CAP LS 302\n Used to list the capabilities supported by the server");
    else if (strncmp(buffer[1], "QUIT", 4) == 0)
        send_msg(client_fd, "IRC /QUIT [:<message>]\n Used to disconnect from the server optionally with a quit message");
    else if (strncmp(buffer[1], "CAP END", 7) == 0)
        send_msg(client_fd, "IRC /CAP END\n Ends the capability negotiation process");
    else if (strncmp(buffer[1], "end server 1234987", 18) == 0)
        send_msg(client_fd, "IRC (admin only) /end server 1234987\n Shuts down the server immediately");	
	else
        send_msg(client_fd, "IRC UNKOWN COMMAND");
    ;
	return;
}*/