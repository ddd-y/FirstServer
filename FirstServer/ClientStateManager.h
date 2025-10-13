#pragma once
#include<set>
#include<vector>
#include<mutex>
#include<shared_mutex>

//tell whether a client is client or a second type server
class ClientStateManager
{
private:
	std::set<int> ClientIDs;
	mutable std::shared_mutex StateManagerLock;
public:
	bool IsClient(int id) const
	{
		std::lock_guard<std::shared_mutex> Lock(StateManagerLock);
		return ClientIDs.find(id) != ClientIDs.end();
	}
	void AddClient(int id) 
	{
		std::unique_lock<std::shared_mutex> Lock(StateManagerLock);
		ClientIDs.insert(id);
	}
	void RemoveClient(int id) 
	{
		std::unique_lock<std::shared_mutex> Lock(StateManagerLock);
		if (ClientIDs.find(id) != ClientIDs.end())
			ClientIDs.erase(id);
	}
};

