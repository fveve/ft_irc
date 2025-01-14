#include "Server.hpp"

void	Server::add_client(void)
{
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int new_client = accept(_socket, (struct sockaddr*)&client_address, &client_len);
    if (new_client == -1) {
        std::cerr << "Failed to accept client: " << strerror(errno) << std::endl;
        return;
    }
    User *user = new User(new_client, client_address);

    client_list[new_client] = user;
    pollfd  client_pollfd;
    client_pollfd.fd = new_client;
    client_pollfd.events = POLLIN;
    client_pollfd.revents = 0;
    fds.push_back(client_pollfd);
}

void	Server::set_nickname(std::string nickname_str, int client_fd)
{
    std::vector<std::string> buffer = parse_request((char *)nickname_str.c_str(), " \r\n", 2);
    
    buffer.erase(buffer.begin());
    std::string str(buffer[0]);

    if (unavailable_nick.find(str) != unavailable_nick.end())
    {
        send_msg(client_list[client_fd]->get_client_fd(), "IRC ERR_NICKNAMEINUSE");
        return ;
    }
    else if (str.length() > 9 || str.empty() || str.find(' ') != std::string::npos)
    {
        std::string msg= "IRC 432 ";

        msg.append(str);
        msg.append(" :Erroneous nickname");
        msg = remove_endl(msg, '\n');
        send_msg(client_list[client_fd]->get_client_fd(), msg);
        return ;
    }
    unavailable_nick.insert(str);
    unavailable_nick.erase(client_list[client_fd]->get_nickname());
    if (!client_list[client_fd]->get_nickname().empty())
    {
        std::string msg = "IRC Nickname successfully changed from " + client_list[client_fd]->get_nickname() + " to " + str;
        send_msg(client_fd, msg);
    }
    client_list[client_fd]->set_nickname(str);
}

void	Server::set_username(std::string username_str, int client_fd)
{
    std::vector<std::string> buffer = parse_request((char *)username_str.c_str(), " :*\r\n", 5);
    
    
    buffer.erase(buffer.begin());
    if (username_str.empty() || buffer.size() < 3)
    {
        send_msg(client_list[client_fd]->get_client_fd(), "IRC ERR_NEEDMOREPARAMS");
        return ;
    }
    else if (!client_list[client_fd]->get_username().empty())
    {
        send_msg(client_list[client_fd]->get_client_fd(), "IRC ERR_ALREADYREGISTRED");
        return ;
    }
    client_list[client_fd]->set_username(buffer);

}

void	Server::auth_client(int client_fd, std::string _password)
{
    std::vector<std::string> buffer = parse_request((char *)_password.c_str(), " :\r\n", 2);
    
    buffer.erase(buffer.begin());
    _password.erase(_password.begin());
    _password = buffer[0];
    if (_password.empty())
    {
        send_msg(client_fd, "IRC ERR_NEEDMOREPARAMS");
		return;
    }
	else if (strcmp(_password.c_str(), password.c_str()))
	{
		send_msg(client_fd, "IRC ERR_BADPASSWORD");
		return;
	}
	send_msg(client_fd, "IRC Welcome !");
    client_list[client_fd]->set_auth(true);
    ;
}