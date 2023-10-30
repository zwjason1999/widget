#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <iostream>
#include <fstream>
#include <mutex>


// 日志等级枚举
enum class LogLevel 
{
    CLOSE,
    DEBUG,
    INFO,
    WARN,
    ERROR
};

namespace tinylog
{

class Logger 
{
private:
    std::mutex mutex_;
    LogLevel logLevel_;  // 日志等级
    std::ostream* logStream_;
    std::ofstream logFileStream_;

public:
    static Logger& GetInstance() 
    {
        static Logger instance;  // 使用静态局部变量确保单例
        return instance;
    }

    // 设置日志等级
    void SetLogLevel(LogLevel level);
    // 设置日志输出文件
    void SetLogFile(std::string logFilePath);
    // 格式化日志
    void Log(LogLevel level, const char* fileName, int lineNumber, const char* format, ...);

private:
    Logger() :logLevel_(LogLevel::DEBUG), logStream_(&std::cerr) {}  // 构造函数私有化以防止外部实例化
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    ~Logger() 
    {
        if (logFileStream_.is_open()) {
            logFileStream_.close();
        }
    }

    std::string GetCurrentTime();
    std::string FormatString(const char* format, va_list args);

};  // Logger



}   // namespace tinylog

void SetLogLevel(LogLevel level);
void SetLogFile(std::string logFilePath);

#define DEBUG(format, ...) tinylog::Logger::GetInstance().Log(LogLevel::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define INFO(format, ...) tinylog::Logger::GetInstance().Log(LogLevel::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define WARN(format, ...) tinylog::Logger::GetInstance().Log(LogLevel::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define ERROR(format, ...) tinylog::Logger::GetInstance().Log(LogLevel::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)


#endif