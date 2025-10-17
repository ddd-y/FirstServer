#include "ProcessPool.h"
#include<chrono>
#include"logger.h"
#include"MyInternet.h"

void ProcessPool::HandleSuspicious(int ProcessID)
{
	auto iter = ValidProcesses.find(ProcessID);
	if (iter != ValidProcesses.end()) 
	{
		auto meta_process = iter->second;
		auto time_now = std::chrono::steady_clock::now();
		double duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_now-meta_process->LastUpdateTime).count();
		//将毫秒转换成秒，符合单位
		duration /= 1000.0;
		if (duration > 2 * UPDATE_DURATION)
		{
			RemoveProcess_private(ProcessID);
		}
	}
}
void ProcessPool::RemoveProcess_private(int ProcessID)
{
	auto it = ValidProcesses.find(ProcessID);
	if (it != ValidProcesses.end())
	{
		metaProcess* proc = it->second;
		ProcessQueue.erase(proc);
		ValidProcesses.erase(it);
		IPMap.erase(proc->IP);
		MyInternet* Reactor = MyInternet::getInstance();
		Reactor->RemoveConnection(proc->relatedfd);
		delete proc;
	}
}

void ProcessPool::AddProcess(metaProcess&& NewProcess)
{
	if(NewProcess.relatedfd == -1 || NewProcess.IP.empty() || NewProcess.Port == 0)
		return;
	std::unique_lock<std::shared_mutex> lock(process_mutex);
	metaProcess* newProc = new metaProcess(std::move(NewProcess));
	int pid = newProc->relatedfd;
	if (ValidProcesses.find(pid) == ValidProcesses.end()) 
	{
		ValidProcesses[pid] = newProc;
		ProcessQueue.emplace(newProc);
		IPMap.emplace(newProc->IP, pid);
	}
	else 
	{
		lock.unlock();
		delete newProc;
	}
}

void ProcessPool::UpDateProcessState(int ProcessID, int NewLoad)
{
	std::unique_lock<std::shared_mutex> lock(process_mutex);
	auto it = ValidProcesses.find(ProcessID);
	if (it != ValidProcesses.end())
	{
		metaProcess* proc = it->second;
		// Remove and reinsert to update its position in the set
		ProcessQueue.erase(proc);
		proc->addLoad(NewLoad);
		ProcessQueue.insert(proc);
	}
}

void ProcessPool::RemoveProcess(int ProcessID)
{
	std::unique_lock<std::shared_mutex> lock(process_mutex);
	RemoveProcess_private(ProcessID);
}

void ProcessPool::HandleClientError(std::string& TheIP)
{
	std::unique_lock<std::shared_mutex> lock(process_mutex);
	auto iter = IPMap.find(TheIP);
	if (iter != IPMap.end()) 
	{
		int SusProcessID = iter->second;
		HandleSuspicious(SusProcessID);
	}
}

std::string ProcessPool::GetProcessIP() const
{
	std::shared_lock<std::shared_mutex> lock(process_mutex);
	if (ProcessQueue.empty())
	{
		return "";
	}
	auto it = ProcessQueue.begin();
	return (*it)->IP;
}


