#include"Handler.h"
#include"ProcessPool.h"
#include"MyInternet.h"
#include"ThreadPool.h"
ClientStateManager Handler::TheClientStateManager;
int main() 
{
    ProcessPool *processPool=new  ProcessPool();
	MyInternet* internet = new MyInternet();
    internet->MainLoop(); // �����¼�ѭ��
    return 0;
}
