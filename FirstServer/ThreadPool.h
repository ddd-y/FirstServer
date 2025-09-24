#pragma once
#include<deque>
#include<vector>
#include<mutex>
#include<thread>
#include <condition_variable>
#include<string>

constexpr int MAX_THREADS = 20;
class ProcessPool;
class Handler;
class MyInternet;
struct TaskData
{
	int client_id; 
	Handler* TheHandler;
};
class ThreadPool
{
private:
	std::mutex task_mutex;
	std::deque<TaskData>task_queue;
	std::vector<std::thread> threads;
	TaskData GetTask();
	std::condition_variable task_cv;
	//the condition to stop the thread pool
	bool stop = false;
public:
	//use to add task
	void AddTask(TaskData task);
	ThreadPool();
	// Each thread runs this function
	void ThreadRun();
};

