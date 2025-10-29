#pragma once
#include"metaProcess.h"
#include<set>
#include<map>
#include<vector>
#include<string>
#include<mutex>
#include<shared_mutex>
#include<queue>

constexpr const int COMPARE_NUM = 3;
enum ProcessEvent
{
	PROCESS_ADDPOOL,
	PROCESS_CHANGESTATE,
	PROCESS_REMOVE
};
class MyInternet;
class ProcessPool
{
private:
	//used to store Valid processes
	std::map<int, metaProcess*> ValidProcesses;
	//利用IP来找对应的fd
	std::map<std::string, int> IPMap;
	mutable std::shared_mutex process_mutex;
	std::set<metaProcess*, CompareMetaProcess> ProcessQueue;
	//客户端发来的可能会出错的Process
	std::set<int> SuspiciousProcess;
	void HandleSuspicious(int ProcessID);
	void RemoveProcess_private(int ProcessID);

	ProcessPool()=default;
	static ProcessPool instance;
public:
	static ProcessPool* getInstance()
	{
		return &instance;
	}
	//add a new process to the pool
	void AddProcess(metaProcess&& NewProcess);
	void UpDateProcessState(int ProcessID, int NewLoad);
	void RemoveProcess(int ProcessID);
	void HandleClientError(std::string& TheIP);
	//choose the process with the least load
	std::string GetProcessIP() const;
};

