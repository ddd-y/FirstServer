#pragma once
#include"ClientStateManager.h"
#include<string>
enum HandlerState
{
	READING,
	WRITING
};
constexpr const char* CLIENT_REQUEST = "Request";
constexpr const char* SERVER_JOIN = "Join";
constexpr const char* SERVER_UPDATE = "Update";
constexpr const int MAX_COMMAND_LENGTH = 7;
class ProcessPool;
class MyInternet;
class ThreadPool;
class metaProcess;
class Handler
{
private:
	HandlerState TaskState;
	int client_fd;

	void HandleRead();
	void HandleWrite();

	void (Handler::* CurrentHandle)();

	ProcessPool* TheProcessPool;

	MyInternet* TheReactor;
	ThreadPool* TheThreadPool;

	//use conn_fd to find the corresponding metaProcess
	metaProcess getMetaProcessByInfo(int conn_fd);
public:
	//use to know the exact logic of the handler
	static ClientStateManager TheClientStateManager;

	Handler(int fd,HandlerState TheState,MyInternet*NewReactor,ThreadPool*NewThreadPool);
	void StartThread();
	void Handle();
};

