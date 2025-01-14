#include "Server.hpp"

void Server::handle_mod(std::string buffer, int client_fd)
{
	std::vector<std::string> tab = parse_request((char *)buffer.c_str(), " \r\n", 4);

	if (tab.size() < 3 || tab.size() > 4)
	{
		send_msg(client_fd, "Syntaxe : MODE <target> <modes> [<mode params>]");
		return;
	}
	std::string mode(tab[2]);
	if (channel_list.find(tab[1]) == channel_list.end())
	{
		send_msg(client_fd, "IRC ERR_CHANNELDOESNTEXIST");
		;
		return;
	}
	else if (mode != "+b" && channel_list[tab[1]]->get_operator() != client_fd)
	{
		send_msg(client_fd, "IRC ERR_DOESTHAVEOPRIGHTS");
		return;
	}
	if (mode == "+i")
	{
		if (!channel_list[tab[1]]->is_invite_only())
		{
			std::string msg = channel_list[tab[1]]->get_name() + "IRC Channel was set to invite only";
			channel_list[tab[1]]->set_invite_only(true);
			channel_list[tab[1]]->send_to_all(0, msg);
		}
		else
		{
			std::string msg = channel_list[tab[1]]->get_name() + "IRC Channel was set to open";
			channel_list[tab[1]]->set_invite_only(false);
			channel_list[tab[1]]->send_to_all(0, msg);
		}
	}
	else if (mode == "+t")
	{
		if (channel_list[tab[1]]->get_operator() == client_fd)
		{
			if (channel_list[tab[1]]->get_topic_changement() == false)
			{
				channel_list[tab[1]]->set_topic_changement(true);
				send_msg(client_fd, "IRC Topic changement is open");
			}
			else
			{
				channel_list[tab[1]]->set_topic_changement(false);
				send_msg(client_fd, "IRC Topic changement is close");
			}
		}
		else
		{
			send_msg(client_fd, "IRC you are not the operator");
			;
			return ;
		}

	}
	else if (mode == "+k")
	{

		channel_list[tab[1]]->set_password(client_fd, tab[3]);
        std::cout << "Change Key :" << (tab.size() > 3 ? tab[3] : "*no params*") << std::endl;
	}
	else if (mode == "+o")
	{
		if (tab.size() > 3)
		{
			std::string msg = "IRC You are setting : ";
			msg += tab[3];
			msg += " as a new operator";
			send_msg(client_fd, msg);
			channel_list[tab[1]]->set_operator(client_fd, tab[3]);
			client_list[client_fd]->remove_op_channel(channel_list[tab[1]]);
		}
		else
			send_msg(client_fd, "IRC ERROR, USAGE : /MODE #channel -o <USER>");
		
	}
	else if (mode == "+l")
	{
		if (tab.size() > 3)
			channel_list[tab[1]]->set_user_limit(std::atoi(tab[3].c_str()));
		else
			channel_list[tab[1]]->set_user_limit(0);
	}
	else if (mode != "+b")
	{
		std::string msg = "IRC Unknown flag : ";
		msg += tab[2];
    	send_msg(client_fd, msg);
	}
    return;
}
