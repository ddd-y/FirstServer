#include "ThreadPool.h"
#include"ProcessPool.h"
#include"Handler.h"
#include"MyInternet.h"
#include<unistd.h>
TaskData ThreadPool::GetTask()
{
    if (!task_queue.empty()) 
    {
        TaskData task = task_queue.front();
        task_queue.pop_front();
        return task;
    }
	return TaskData{ -1,  nullptr}; // Default task if none are available
}


void ThreadPool::AddTask(TaskData task)
{
    std::lock_guard<std::mutex> lock(task_mutex);
    task_queue.push_back(task);
	task_cv.notify_one();
}

ThreadPool::ThreadPool()
{
    for (int i = 0; i < MAX_THREADS; ++i)
    {
        threads.emplace_back(&ThreadPool::ThreadRun, this);
    }
}


void ThreadPool::ThreadRun()
{
    while (!stop) 
    {  
        std::unique_lock<std::mutex> lock(task_mutex);
        task_cv.wait(lock, [this]() {
            return !task_queue.empty() || stop;
            });

        if (stop) {
            break;
        }
        TaskData thetaskdata = GetTask();
        lock.unlock(); 
        if (thetaskdata.client_id != -1)
        {
            Handler* thehandler = thetaskdata.TheHandler;
            if(thehandler)
				thehandler->Handle();
			delete thehandler;
        }
    }
}
