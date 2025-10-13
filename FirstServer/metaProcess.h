#pragma once
#include <netinet/in.h>
#include<string>
#include<cstring>
#include<arpa/inet.h>
#include<array>
#include<chrono>
//Predict的最小精度
constexpr const double MIN_PREDICT = 0.001;

//第二类服务器UPDATE的间隔,单位是秒
constexpr const int UPDATE_DURATION = 3;

//当间隔时间小于这个值的时候，不拿来算变化率，防止变化的跳跃
constexpr const double UPDATE_DURATION_MIN = 500.0;

constexpr const double PRE_PERCENTAGE = 0.5;
//预测算法：指数移动平均，类似于TCP的那个
class metaProcess
{
public:
	//上一次更新的时间
	std::chrono::steady_clock::time_point LastUpdateTime;
	int relatedfd;
	std::string IP;
	//预测Load的变化率，单位是Load每秒,但利用毫秒来计算用以提高精度
	double PredictLoadChange = 0.0;
	double PredictLoad = 0.0;

	int LastLoad=0;
	uint16_t Port;
	metaProcess() :relatedfd(-1), IP(""), Port(0){}
	metaProcess(int newrealtedfd, std::string newstring, uint16_t newPort) : relatedfd(newrealtedfd),
		IP(std::move(newstring)), Port(newPort)
	{
		LastUpdateTime= std::chrono::steady_clock::now();
	}
	metaProcess(metaProcess&& other) noexcept
		: LastUpdateTime(std::move(other.LastUpdateTime)),relatedfd(other.relatedfd), IP(std::move(other.IP)),
		Port(other.Port){
		other.relatedfd = -1;
		other.Port = 0;
		other.PredictLoad = 0.0;
	}
	void addLoad(int newLoad);
};

struct CompareMetaProcess {
	bool operator()(const metaProcess* a,const metaProcess* b) const{
		return a->PredictLoad < b->PredictLoad;
	}
};

