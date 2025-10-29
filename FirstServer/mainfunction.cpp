#include"Handler.h"
#include"ProcessPool.h"
#include"MyInternet.h"
#include"ThreadPool.h"
#include"logger.h"
ClientStateManager Handler::TheClientStateManager;
ThreadPool ThreadPool::instance;
ProcessPool ProcessPool::instance;
bool MyInternet::IfInited = false;
MyInternet* MyInternet::instance = nullptr;
int main() 
{
    init_logger("logs/FirstServer.log", spdlog::level::debug);
	MyInternet::Init();
	MyInternet* internet = MyInternet::getInstance();
    internet->MainLoop(); // 启动事件循环
    return 0;
}
