#pragma once
#include<deque>
#include<vector>
#include<mutex>
#include<thread>
#include <condition_variable>
#include<set>
#include<string>
#include<map>
#include<atomic>

constexpr int MAX_THREADS = 20;
constexpr int MAX_TOOLONG = 5;
class ProcessPool;
class Handler;
class MyInternet;
struct TaskData
{
	int client_id; 
	ProcessPool* tProcessPool;
	MyInternet* tReactor;
	bool readorwrite;
};
class ThreadPool
{
private:
	std::mutex task_mutex;
	std::deque<TaskData>task_queue;
	std::vector<std::thread> threads;
	std::set<Handler*> ActiveHandler;
	//未完全完成任务的handler
	std::map<int, Handler*> NotComHandlers;
	std::set<Handler*> DeleteHandlers;
	std::mutex ActiveLock;
	std::mutex DeleteLock;
	std::mutex NotComLock;
	TaskData GetTask();
	std::condition_variable task_cv;
	std::condition_variable TooLong_cv;
	std::condition_variable Delete_cv;
	//the condition to stop the thread pool
	std::atomic<bool> stop = false;
	void CleanOverTime();
	void AddDelete(Handler* delete_handler);
public:
	//use to add task
	void pDeleteHandler();
	void RunHandler(int fd,bool readorwrite,ProcessPool* tprocesspool, MyInternet* newReactor);
	ThreadPool();
	~ThreadPool();
	// Each thread runs this function
	void ThreadRun();
};

