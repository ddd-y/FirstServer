#pragma once
#include <netinet/in.h>
#include<string>
#include<cstring>
#include<arpa/inet.h>
class metaProcess
{
public:
	//use for first type server to communicate with second type server
	int relatedfd;
	std::string IP;
	uint16_t Port;

	metaProcess() :relatedfd(-1), IP(""), Port(0){}
	metaProcess(int newrealtedfd, std::string newstring, uint16_t newPort) : relatedfd(newrealtedfd), IP(std::move(newstring)), Port(newPort){}
};

