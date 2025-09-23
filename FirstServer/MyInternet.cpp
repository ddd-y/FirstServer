#include "MyInternet.h"
#include <sys/socket.h>    // 核心socket函数（socket、bind、listen等）
#include <netinet/in.h>    // 网络地址结构（sockaddr_in等）
#include <arpa/inet.h>     // IP地址转换函数（inet_pton、inet_ntoa等）
#include <unistd.h>        // 关闭文件描述符（close）
#include <errno.h>        
#include<cstring>
#include<iostream>
constexpr auto SERVER_IP = "127.0.0.1";
constexpr auto SERVER_PORT = 8888;
void MyInternet::PreConnect()
{
	Thefd = socket(AF_INET, SOCK_STREAM, 0);
	if (Thefd == -1) 
	{
		// 处理错误
		perror("socket creation failed");
		return;
	}
	struct sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t len = sizeof(server_addr);
	int ret = bind(Thefd, (struct sockaddr*)&server_addr, len);
	if (ret == -1)
		perror("bind failed");
	listen(Thefd, 5);
}

void MyInternet::Connect()
{
}

void MyInternet::Disconnect()
{
}
