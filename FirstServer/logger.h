#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
extern std::shared_ptr<spdlog::logger> logger;
void init_logger(const std::string& log_path = "logs/FirstServer.log",
    spdlog::level::level_enum level = spdlog::level::info);
#define LOG_INFO(...)   logger->info(__VA_ARGS__)
#define LOG_WARN(...)   logger->warn(__VA_ARGS__)
#define LOG_ERROR(...)  logger->error(__VA_ARGS__)
#define LOG_DEBUG(...)  logger->debug(__VA_ARGS__)

