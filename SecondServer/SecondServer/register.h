#pragma once
#include<string>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<iostream>
#include<algorithm>
#include<cstring>
#include<unistd.h>

//注册到第一类服务器的注册器



class Register
{
public:
	Register(const std::string& ip_,const short port);
	~Register();
	
	short start();
	void heart_info_to_server();

private:
	sockaddr_in saddr;
	int fd;
	char buffer[1024];
	const std::string first_server_ip;
	const short first_server_port;
	
};

