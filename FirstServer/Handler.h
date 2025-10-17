#pragma once
#include"ClientStateManager.h"
#include<string>
#include<chrono>

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

constexpr const char PARTITION_CHAR = '.';
constexpr const int MAX_INCOMPMES_SIZE = 256;

//最长存在时间,单位为秒
constexpr const int MAX_LIFE = 60;
class ProcessPool;
class MyInternet;
class ThreadPool;
class metaProcess;
class Handler
{
private:
	std::chrono::steady_clock::time_point LastHandleTime;
	//未拼接完成的消息
	std::string IncompleteMes;

	HandlerState TaskState;
	int client_fd;

	ProcessPool* TheProcessPool;

	MyInternet* TheReactor;

	//use conn_fd to find the corresponding metaProcess
	metaProcess getMetaProcessByInfo(int conn_fd);

	bool HandleRead();
	bool HandleWrite();

	bool (Handler::* CurrentHandle)();

	//the son function of handleread

	void HandleCR(std::string& command);//handle client request
	void HandleSJ(std::string& command);//handle second type server join
	void HandleSU(std::string& command);//handle second type server update
	void HandleSL(std::string& command);//handle second type server leave when second type server exit
	void HandleFE(std::string& command);//handle second type server error found by client
	void HandleInvalid();//handle invalid command

	//封装，根据消息的首字符来处理
	void HandleCompleteMes(std::string& command);


public:
	//use to know the exact logic of the handler
	static ClientStateManager TheClientStateManager;

	Handler(int fd, HandlerState TheState);
	bool Handle();
	//处理时间是否过长
	bool IfTooLong(std::chrono::steady_clock::time_point& current_time);

	//封装，解除连接
	void CleanupConnection();
	int GetFd() { return client_fd; }
};

