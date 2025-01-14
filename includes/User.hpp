#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <map> 
#include <vector>
#include <poll.h>

class Channel;

class User
{
	private:
		std::string							nickname;
		std::string							username;
		std::string							fullname;
		std::string							userBuffer;
		bool								authentification;
		bool								_operator;
		int									client_fd;
		struct sockaddr_in 					client_adress;
		std::map<std::string, Channel *>	channel_joined;
		std::map<std::string, Channel *>	channel_invited;
		std::map<std::string, Channel *>	isOp_onChannel;

	public:
	/*-------Construction-------*/
	User(int client_fd, struct sockaddr_in client_adress);
	~User();

	/*------Setters-----*/
		
		void								set_auth(bool auth);
		void								set_nickname(std::string _nickname);
		void								set_username(std::vector<std::string> _username);

		void								set_invitedChannel( Channel *channel);

	/*------Getters----*/
		std::string							get_nickname() const;
		std::string							get_username() const;
		std::string							get_fullname() const;
		bool								get_auth() const;
		bool								get_operator(Channel *channel) const;
		int									get_client_fd() const;
		struct sockaddr_in					get_client_adress() const;
		std::map<std::string, Channel *>	get_channel_list() const;
		std::string							get_userBuffer();

		std::map<std::string, Channel *>	get_channel_invited() const;
		void								remove_invitation(std::string channel);
	/*-----Others-----*/
		void								user_buffer(char *new_buffer);
		void								remove_channel(std::string channel_name);
		void								add_channel( Channel *channel);
		bool								is_joined(std::string channel);
		


		/*------Operator function-----*/
		void								set_operator(Channel * channel);
		bool								get_operator() const;
		bool								get_op(const Channel *channel);
		std::map<std::string, Channel *>	get_isOp_onChannel();
		void								remove_op_channel(Channel *channel);

};
void								send_msg(int client_fd, std::string message);

#include "Channel.hpp"

