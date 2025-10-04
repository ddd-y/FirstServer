#include "ProcessPool.h"

void ProcessPool::AddProcess(metaProcess&& NewProcess)
{
	if(NewProcess.relatedfd == -1 || NewProcess.IP.empty() || NewProcess.Port == 0)
		return;
	std::lock_guard<std::mutex> lock(process_mutex);
	metaProcess* newProc = new metaProcess(std::move(NewProcess));
	int pid = newProc->relatedfd;
	if (ValidProcesses.find(pid) == ValidProcesses.end()) {
		ValidProcesses[pid] = newProc;
		ProcessQueue.emplace(newProc);
	}
	else 
	{
		delete newProc;
	}
}

void ProcessPool::UpDateProcessState(int ProcessID, int NewLoad)
{
	std::lock_guard<std::mutex> lock(process_mutex);
	auto it = ValidProcesses.find(ProcessID);
	if (it != ValidProcesses.end())
	{
		metaProcess* proc = it->second;
		// Remove and reinsert to update its position in the set
		ProcessQueue.erase(proc);
		proc->Load = NewLoad;
		ProcessQueue.insert(proc);
	}
}

void ProcessPool::RemoveProcess(int ProcessID)
{
	std::lock_guard<std::mutex> lock(process_mutex);
	auto it = ValidProcesses.find(ProcessID);
	if (it != ValidProcesses.end())
	{
		metaProcess* proc = it->second;
		ProcessQueue.erase(proc);
		ValidProcesses.erase(it);
		delete proc;
	}
}

std::string ProcessPool::GetProcessIP()
{
	std::lock_guard<std::mutex> lock(process_mutex);
	if (ProcessQueue.empty())
		return "None Server Available";
	//local prediction
	auto it = ProcessQueue.begin();
	int newLoad = (*it)->Load + 1;
	UpDateProcessState((*it)->relatedfd, newLoad);
	return (*it)->IP;
}

