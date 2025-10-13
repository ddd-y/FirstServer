#pragma once
#include"ClientStateManager.h"
#include<string>
enum HandlerState
{
	READING,
	WRITING
};
//客户端请求第二类服务器
constexpr const char* CLIENT_REQUEST = "Request";
constexpr const int REQUEST_LENGTH = 7;
//客户端发现服务器错误
constexpr const char* CLIENT_FIND_ERROR = "FindE";
constexpr const int FIND_ERROR_LENGTH = 5;
//第二类服务器请求加入
constexpr const char* SERVER_JOIN = "Join";
constexpr const int JOIN_LENGTH = 4;
//第二类服务器请求更新
constexpr const char* SERVER_UPDATE = "Update";
constexpr const int UPDATE_LENGTH = 6;
//第二类服务器请求离开
constexpr const char* SERVER_LEAVE = "Leave";
constexpr const int LEAVE_LENGTH = 5;
class ProcessPool;
class MyInternet;
class ThreadPool;
class metaProcess;
class Handler
{
private:
	HandlerState TaskState;
	int client_fd;

	ProcessPool* TheProcessPool;

	MyInternet* TheReactor;
	ThreadPool* TheThreadPool;

	//use conn_fd to find the corresponding metaProcess
	metaProcess getMetaProcessByInfo(int conn_fd);

	void HandleRead();
	void HandleWrite();

	void (Handler::* CurrentHandle)();

	//the son function of handleread

	void HandleCR(std::string&& command);//handle client request
	void HandleSJ(std::string&& command);//handle second type server join
	void HandleSU(std::string&& command);//handle second type server update
	void HandleSL(std::string&& command);//handle second type server leave when second type server exit
	void HandleFE(std::string&& command);//handle second type server error found by client
	void HandleInvalid();//handle invalid command

public:
	//use to know the exact logic of the handler
	static ClientStateManager TheClientStateManager;

	Handler(int fd,HandlerState TheState,MyInternet* NewReactor,ThreadPool* NewThreadPool,ProcessPool* NewProcessPool);
	void StartThread();
	void Handle();
};

