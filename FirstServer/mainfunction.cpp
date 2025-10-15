#include"Handler.h"
#include"ProcessPool.h"
#include"MyInternet.h"
#include"ThreadPool.h"
#include"logger.h"
ClientStateManager Handler::TheClientStateManager;
int main() 
{
    init_logger("logs/FirstServer.log", spdlog::level::debug);
	MyInternet* internet = new MyInternet();
    internet->MainLoop(); // 启动事件循环
    return 0;
}
