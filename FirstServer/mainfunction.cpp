#include"Handler.h"
#include"ProcessPool.h"
#include"MyInternet.h"
#include"ThreadPool.h"
#include<iostream>

ClientStateManager Handler::TheClientStateManager;
int main() 
{   
   
    ProcessPool *processPool=new  ProcessPool();
	MyInternet* internet = new MyInternet();
    std::cout << "start successfully----------------" << std::endl;
    std::cout << "running-------" << std::endl;
    internet->MainLoop(); // 启动事件循环

    std::cout << "server close...." << std::endl;
   
  
    return 0;
}
