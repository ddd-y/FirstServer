#pragma once

#include"spdlog/async.h"
#include"spdlog/spdlog.h"
#include<memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class MyLogger
{
public:
	static MyLogger& get_instance()
	{
		static MyLogger instance;
		return instance;
	}
	~MyLogger();


private:
	struct loggers
	{
		std::shared_ptr<spdlog::logger> sync_logger;
		std::shared_ptr<spdlog::logger> async_logger;
	};

	MyLogger();
	MyLogger(const MyLogger&) = delete;
	MyLogger(const MyLogger&&) = delete;
	MyLogger& operator=(const MyLogger&) = delete;
	MyLogger& operator=(const MyLogger&&) = delete;

	

	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<spdlog::logger> slogger_;
	std::vector<spdlog::sink_ptr> sinks_;

public:
	

	loggers get() const;


};


#define LOG_INFO(...)   MyLogger::get_instance().get().async_logger->info(__VA_ARGS__); MyLogger::get_instance().get().sync_logger->info(__VA_ARGS__)
#define LOG_DEBUG(...)  MyLogger::get_instance().get().async_logger->debug(__VA_ARGS__); MyLogger::get_instance().get().sync_logger->debug(__VA_ARGS__)
#define LOG_WARN(...)   MyLogger::get_instance().get().async_logger->warn(__VA_ARGS__); MyLogger::get_instance().get().sync_logger->warn(__VA_ARGS__)
#define LOG_ERROR(...)  MyLogger::get_instance().get().async_logger->error(__VA_ARGS__); MyLogger::get_instance().get().sync_logger->error(__VA_ARGS__)