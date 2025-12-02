#include"Handler.h"
#include"ProcessPool.h"
#include"MyInternet.h"
#include"ThreadPool.h"
#include"logger.h"
#include"crow_api.h"

ClientStateManager Handler::TheClientStateManager;
ThreadPool ThreadPool::instance;
ProcessPool ProcessPool::instance;
bool MyInternet::IfInited = false;
MyInternet* MyInternet::instance = nullptr;
int main() 
{	

	std::thread server_thread([]() {
		run_crow_server(8081);
		});

	MyInternet::Init();
	MyInternet* internet = MyInternet::getInstance();
    internet->MainLoop(); // 启动事件循环


    return 0;
}
