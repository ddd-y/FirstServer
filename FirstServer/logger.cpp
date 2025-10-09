#include "logger.h"
#include <filesystem>
namespace fs = std::filesystem;
std::shared_ptr<spdlog::logger> logger;
void init_logger(const std::string& log_path, spdlog::level::level_enum level) 
{
    fs::path log_dir = fs::path(log_path).parent_path();
    if (!log_dir.empty() && !fs::exists(log_dir)) 
    {
        fs::create_directories(log_dir);
    }
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, true);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%s:%#] %v");
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{ file_sink, console_sink });
    logger->set_level(level);
    logger->flush_on(spdlog::level::debug);
}
