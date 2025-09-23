#pragma once
#include<array>
#include<vector>
class MyInternet
{
private:
	int Thefd;
	std::array<char,1024> buffer;
public:
	//prepare for connection before actually connecting
	MyInternet() :Thefd(-1){}
	void PreConnect();
	void Connect();
	void Disconnect();
};

