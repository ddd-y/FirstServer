#pragma once
#include"metaProcess.h"
#include<map>
#include<vector>
#include<string>
#include<mutex>

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
	//used to store all metaprocesses
	std::map<int, metaProcess> Processes;
	std::vector<int> ProcessIDs;

	//the first int is the process id, the second int is the load of the process
	std::map<int, int> ProcessLoads;
	std::mutex process_mutex;
public:
	//add a new process to the pool
	void AddProcess(const metaProcess& NewProcess);
	void UpDateProcessState(int ProcessID, int NewLoad);
	//choose the process with the least load
	std::string GetProcessIP();
};

