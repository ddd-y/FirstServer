#pragma once
#include"metaProcess.h"
#include<set>
#include<map>
#include<vector>
#include<string>
#include<mutex>
#include<queue>

enum ProcessEvent
{
	PROCESS_ADDPOOL,
	PROCESS_CHANGESTATE,
	PROCESS_REMOVE
};
class ProcessPool
{
private:
	//total number of processes
	int ProcessNum;
	//used to store Valid processes
	std::map<int, metaProcess*> ValidProcesses;

	std::mutex process_mutex;
	std::set<metaProcess*, CompareMetaProcess> ProcessQueue;
public:
	//add a new process to the pool
	void AddProcess(metaProcess&& NewProcess);
	void UpDateProcessState(int ProcessID, int NewLoad);
	void RemoveProcess(int ProcessID);
	//choose the process with the least load
	std::string GetProcessIP();
};

