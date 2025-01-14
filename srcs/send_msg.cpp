#include "Server.hpp"

std::string    remove_endl(std::string str , char delim)
{
    while (str.find(delim) != std::string::npos)
        str.erase(str.find(delim), 1);
    return (str);
}

void	Server::channel_msg(int client_fd, std::string channel_name, std::string msg)
{
    if (channel_list.find(channel_name) == channel_list.end())
    {
        send_msg(client_fd, "IRC 403 ERR_NOSUCHCHANNEL");
        return ;
    }
    if (!client_list[client_fd]->is_joined(channel_name))
    {
        send_msg(client_fd, "IRC 442 ERR_NOTONCHANNEL");
        return ;
    }
    std::string nickname = client_list[client_fd]->get_nickname();
    if (channel_list[channel_name]->get_operator() == client_fd)
    {
        nickname.insert(0, "@");
    }
	std::string to_send = ":" + client_list[client_fd]->get_nickname() + "!~" + client_list[client_fd]->get_username() + "@host PRIVMSG " + channel_name + " :" + msg;
    to_send = remove_endl(to_send, '\n');
    channel_list[channel_name]->send_to_all(client_fd, to_send);
}

void	Server::privmsg(int client_fd, std::string demand)
{
    if (!client_list[client_fd]->get_auth())
    {
        send_msg(client_fd, "IRC Error: you need to be authenticated first");
        return ;
    }
    std::vector<std::string> buffer = parse_request((char *)demand.c_str(), " \r\n", 3);

    if (buffer.size() < 3)
    {
        if (buffer.size() == 2)
            send_msg(client_fd, "IRC 411 ERR_NORECIPIENT");
        else
            send_msg(client_fd, "IRC 412 ERR_NOTEXTTOSEND");
        return ;
    }
	if (buffer[1][0] == '#')
    {
		channel_msg(client_fd, buffer[1], buffer[2]);
        return ;
    }
    std::map<int, User *>::iterator it = client_list.begin();

    for (; it != client_list.end() && strcmp(it->second->get_nickname().c_str(), buffer[1].c_str());)
        it++;
    if (it == client_list.end())
    {
        send_msg(client_fd, "IRC 401 ERR_NOSUCHNICK");
        return ;
    }
    if (!client_list[it->first]->get_auth())
    {
        send_msg(client_fd, "IRC Error: receptor needs to be authenticated first");
        return ;
    }
    std::string msg = ":" + client_list[client_fd]->get_nickname() + "!~" + client_list[client_fd]->get_username() + "@host PRIVMSG " + buffer[1] + " :" + buffer[2];
    msg = remove_endl(msg, '\n');
    send_msg(it->first, msg);
    ;
}
