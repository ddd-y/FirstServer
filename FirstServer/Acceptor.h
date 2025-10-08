#pragma once
#include <netinet/in.h>
class MyInternet;
class Acceptor
{
private:
	int listenfd;
	sockaddr_in server_addr;
public:
	Acceptor(int port);
	int GetListenFd() const { return listenfd; }
	void AcceptConnection(MyInternet* TheReactor);
};

