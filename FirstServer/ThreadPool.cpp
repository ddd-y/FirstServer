#include "ThreadPool.h"
#include"ProcessPool.h"
#include"Handler.h"
#include"MyInternet.h"
#include<unistd.h>
#include<chrono>

TaskData ThreadPool::GetTask()
{
    if (!task_queue.empty()) 
    {
        TaskData task = task_queue.front();
        task_queue.pop_front();
        return task;
    }
	return TaskData{ -1,  false};
}


void ThreadPool::CleanOverTime()
{
    while (!stop)
    {
        std::unique_lock<std::mutex> lock(NotComLock);
        TooLong_cv.wait_for(lock, std::chrono::seconds(5), [this]() {
            return !NotComHandlers.empty() || stop;
            });
        if (stop)
            break;
        int cleaned = 0;
        auto current_time = std::chrono::steady_clock::now();
        for (auto iter = NotComHandlers.begin(); iter != NotComHandlers.end() && cleaned < MAX_TOOLONG;)
        {
            Handler* thehandler = iter->second;
            if (thehandler->IfTooLong(current_time))
            {
                thehandler->CleanupConnection();
                AddDelete(thehandler);
                iter = NotComHandlers.erase(iter);
            }
            ++cleaned;
        }
    }
}


void ThreadPool::AddDelete(Handler* delete_handler)
{
    std::lock_guard<std::mutex> lock(DeleteLock);
    DeleteHandlers.insert(delete_handler);
}

void ThreadPool::pDeleteHandler()
{
    std::vector<Handler*> readydelete;
    {
        std::lock_guard<std::mutex> tlock(DeleteLock);
        int size = DeleteHandlers.size();
        int divsize = size / 4;
        size = divsize < 1 ? size : divsize;
        auto iter = DeleteHandlers.begin();
        for (int i = 0; i < size && iter != DeleteHandlers.end(); ++i) 
        {
            readydelete.push_back(*iter);
            ++iter;
        }
        DeleteHandlers.erase(DeleteHandlers.begin(), iter);
    }
    for (auto dhandler : readydelete)
        delete dhandler;
}


void ThreadPool::RunHandler(int fd, bool readorwrite)
{
    TaskData thedata = TaskData{fd,readorwrite};
    std::lock_guard<std::mutex> lock(task_mutex);
    task_queue.push_back(thedata);
    task_cv.notify_one();
}


ThreadPool::ThreadPool()
{
    for (int i = 0; i < MAX_THREADS; ++i)
    {
        threads.emplace_back(&ThreadPool::ThreadRun, this);
    }
    threads.emplace_back(&ThreadPool::CleanOverTime,this);
}

ThreadPool::~ThreadPool()
{
    stop = true;
    task_cv.notify_all();
    TooLong_cv.notify_all(); 
    for (auto& t : threads) 
    {
        if (t.joinable()) 
        {
            t.join(); 
        }
    }
    {
        std::lock_guard<std::mutex> lock(ActiveLock);
        for (auto handler : ActiveHandler)
        {
            delete handler;
        }
        ActiveHandler.clear();
    }

    {
        std::lock_guard<std::mutex> lock(NotComLock);
        for (auto& pair : NotComHandlers)
        {
            delete pair.second;
        }
        NotComHandlers.clear();
    }

    {
        std::lock_guard<std::mutex> lock(DeleteLock);
        for (auto handler : DeleteHandlers)
        {
            delete handler;
        }
        DeleteHandlers.clear();
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
        HandlerState handler_state = thetaskdata.readorwrite ? HandlerState::READING : HandlerState::WRITING;
        if (thetaskdata.client_id != -1)
        {
            Handler* thehandler = nullptr;
            {
                std::lock_guard<std::mutex> lock(NotComLock);
                auto it = NotComHandlers.find(thetaskdata.client_id);
                if (it == NotComHandlers.end())
                {
                    thehandler = new Handler(thetaskdata.client_id, handler_state);
                }
                else
                {
                    thehandler = it->second;
                    NotComHandlers.erase(it);
                }
            }
            bool need_delete = true;
            if (!thehandler)
                continue;

            std::unique_lock<std::mutex> active_lock(ActiveLock);
            ActiveHandler.insert(thehandler);
            active_lock.unlock();

			need_delete=thehandler->Handle();
            if (need_delete)
            {
                AddDelete(thehandler);
                std::lock_guard<std::mutex> slock(ActiveLock);
                ActiveHandler.erase(thehandler);
                continue;
            }
            {
                std::lock_guard<std::mutex> tlock(NotComLock);  // 自动加锁/解锁，更简洁
                NotComHandlers[thehandler->GetFd()] = thehandler;
            }
            {
                std::lock_guard<std::mutex> alock(ActiveLock);  // 缩小锁范围
                ActiveHandler.erase(thehandler);
            }
            TooLong_cv.notify_one();
        }
    }
}
