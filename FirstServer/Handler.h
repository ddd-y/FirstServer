#pragma once
#include"ClientStateManager.h"
#include<string>
enum HandlerState
{
	READING,
	WRITING
};
class ProcessPool;
class MyInternet;
class ThreadPool;
class Handler
{
private:
	int client_fd;
	HandlerState TaskState;

	void HandleRead();
	void HandleWrite();

	void (Handler::* CurrentHandle)();

	ProcessPool* TheProcessPool;

	MyInternet* TheReactor;
	ThreadPool* TheThreadPool;
	const std::string CLENT_REQUEST = "Request";
	const std::string SERVER_JOIN = "Join";

	std::string getPeerIP(int conn_fd,int &getport);
public:
	//use to know the exact logic of the handler
	static ClientStateManager TheClientStateManager;

	Handler(int fd,HandlerState TheState,MyInternet*NewReactor,ThreadPool*NewThreadPool);
	void StartThread();
	void Handle();
};

