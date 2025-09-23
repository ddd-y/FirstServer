#include "MyInternet.h"
#include <sys/socket.h>    // ����socket������socket��bind��listen�ȣ�
#include <netinet/in.h>    // �����ַ�ṹ��sockaddr_in�ȣ�
#include <arpa/inet.h>     // IP��ַת��������inet_pton��inet_ntoa�ȣ�
#include <unistd.h>        // �ر��ļ���������close��
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
		// �������
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
