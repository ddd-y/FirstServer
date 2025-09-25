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
	int Load = 0;
	metaProcess() :relatedfd(-1), IP(""), Port(0){}
	metaProcess(int newrealtedfd, std::string newstring, uint16_t newPort) : relatedfd(newrealtedfd), IP(std::move(newstring)), Port(newPort){}
	metaProcess(metaProcess&& other) noexcept
		: relatedfd(other.relatedfd), IP(std::move(other.IP)),
		Port(other.Port), Load(other.Load) {
		other.relatedfd = -1;
		other.Port = 0;
		other.Load = 0;
	}
};

struct CompareMetaProcess {
	bool operator()(const metaProcess* a,const metaProcess* b) {
		return a->Load < b->Load;
	}
};

