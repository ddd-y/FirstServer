#include "ProcessPool.h"

void ProcessPool::AddProcess(const metaProcess& NewProcess)
{
	std::lock_guard<std::mutex> lock(process_mutex);
	ProcessIDs.push_back(NewProcess.relatedfd);
	Processes[NewProcess.relatedfd] = NewProcess;
	ProcessLoads[NewProcess.relatedfd] = 0;
	ProcessNum = ProcessIDs.size();
}

void ProcessPool::UpDateProcessState(int ProcessID, int NewLoad)
{
	std::lock_guard<std::mutex> lock(process_mutex);
	if (Processes.find(ProcessID) != Processes.end())
	{
		ProcessLoads[ProcessID] = NewLoad;
	}
}

std::string ProcessPool::GetProcessIP()
{
	std::lock_guard<std::mutex> lock(process_mutex);
	int minLoad = INT32_MAX;
	int minProcessID = -1;
	//后面可以把这个改成优先队列
	for (const auto& [pid, load] : ProcessLoads)
	{
		if (load < minLoad)
		{
			minLoad = load;
			minProcessID = pid;
		}
	}
	if (minProcessID != -1)
	{
		const metaProcess& proc = Processes[minProcessID];
		++ProcessLoads[minProcessID];
		return proc.IP + ":" + std::to_string(proc.Port);
	}
	return "";
}

