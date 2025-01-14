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
#include <unistd.h>
#include <set>
#include <sstream>
#include <exception>
#include <cerrno>
#include <algorithm>

#include "Channel.hpp"

class Server
{
	private:
		int									_socket;
		int									port;
   		struct sockaddr_in 					server_address;
		std::string							password;
		std::map<int, User*>				client_list;
		std::map<std::string, Channel *>	channel_list;
		std::vector<struct pollfd>			fds;
		std::set<std::string>				unavailable_nick;
		
	public:
		Server(int port, std::string _password);
		void								start_server(void);
		std::vector<std::string>					parse_request(char *buffer, const char *delim, int words);
		void								handle_request(void);
		void								add_client(void);
		void								invite_user(int client_fd, std::string demand);
		void								change_topic(int client_fd, std::string demand);
		void								remove_client(std::map<int, User *>::iterator it, int x);
		void								auth_client(int client_fd, std::string _password);
		void								set_nickname(std::string nickname_str, int client_fd);
		void								set_username(std::string username_str, int client_fd);
		void								join_channel(std::string command, int client_fd);
		void								quit_channel(std::string channel_name, int client_fd);
		void								privmsg(int client_fd, std::string demand); 
		void								channel_msg(int client_fd, std::string channel_name, std::string msg);
		void								handle_mod(std::string buffer, int client_fd);
		void   								handle_each_channel(std::string channel_name, int client_fd);
		int									get_socket() const;
		void								hashCommand(char* _buffer, std::map<int, User *>::iterator it, int x);
		struct sockaddr_in					get_server_adress() const;
		std::string							get_password() const;
		std::map<int, User*>				get_client_list() const;
		std::map<std::string, Channel *>	get_channel_list() const;
		std::vector<struct pollfd>			get_fds() const;
		std::set<std::string>				get_unavilable_nick() const;
		int									check_invitation(int client_fd, std::string channel_name);
		~Server();

		/*----Nucleocherrry's functions-------*/
		int									get_client_fd_by_nickname(std::string _nickname);
		void								kick_user(std::string demand, int client_fd);
};

void								send_msg(int client_fd, std::string message);
std::string    						remove_endl(std::string str , char delim);



