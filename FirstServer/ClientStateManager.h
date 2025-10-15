#pragma once
#include<set>
#include<vector>

//tell whether a client is client or a second type server
class ClientStateManager
{
private:
	std::set<int> ClientIDs;
public:
	bool IsClient(int id) 
	{
		return ClientIDs.find(id) != ClientIDs.end();
	}
	void AddClient(int id) 
	{
		ClientIDs.insert(id);
	}
	void RemoveClient(int id) 
	{
		if (ClientIDs.find(id) != ClientIDs.end())
			ClientIDs.erase(id);
	}
};

