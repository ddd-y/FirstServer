#include "xlogger.h"

#include <spdlog/sinks/base_sink.h>
#include <mutex>

//template<typename Mutex>
//class custom_rotating_sink : public spdlog::sinks::base_sink<Mutex> {
//private:
//    std::string base_filename_;
//    size_t max_size_;
//    size_t current_size_;
//    std::FILE* file_;
//
//    void rotate_if_needed() {
//        if (current_size_ >= max_size_) {
//            // 执行回滚
//            std::fclose(file_);
//
//            // 生成带时间戳的新文件名
//            auto now = std::chrono::system_clock::now();
//            auto time_t = std::chrono::system_clock::to_time_t(now);
//            std::tm tm;
//            localtime_r(&time_t, &tm);
//
//            char buffer[64];
//            std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &tm);
//            std::string new_name = base_filename_ + "." + buffer;
//
//            // 重命名当前文件
//            std::rename(base_filename_.c_str(), new_name.c_str());
//
//            // 重新打开文件
//            file_ = std::fopen(base_filename_.c_str(), "a");
//            current_size_ = 0;
//        }
//    }
//
//protected:
//    void sink_it_(const spdlog::details::log_msg& msg) override {
//        spdlog::memory_buf_t formatted;
//        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
//
//        rotate_if_needed();
//
//        auto size = std::fwrite(formatted.data(), sizeof(char), formatted.size(), file_);
//        if (size > 0) {
//            current_size_ += size;
//        }
//    }
//
//    void flush_() override {
//        std::fflush(file_);
//    }
//
//public:
//    custom_rotating_sink(const std::string& filename, size_t max_size)
//        : base_filename_(filename), max_size_(max_size), current_size_(0) {
//        file_ = std::fopen(filename.c_str(), "a");
//        if (!file_) {
//            throw spdlog::spdlog_ex("Failed to open file");
//        }
//    }
//
//    ~custom_rotating_sink() {
//        if (file_) {
//            std::fclose(file_);
//        }
//    }
//};

//using custom_rotating_sink_mt = custom_rotating_sink<std::mutex>;

MyLogger::~MyLogger()
{
	spdlog::drop_all();
}


MyLogger::MyLogger()
{
	
	spdlog::init_thread_pool(8192, 1);
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/combined.log", false);

    std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };

	slogger_ = std::make_shared<spdlog::logger>("sync_logger",console_sink);
	slogger_->set_level(spdlog::level::debug);
	slogger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");


	logger_ = std::make_shared<spdlog::async_logger>("async_logger", file_sink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	logger_->set_level(spdlog::level::debug);
	logger_->flush_on(spdlog::level::debug);
	logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
}

MyLogger::loggers MyLogger::get() const
{
	return { slogger_,logger_ };
}
