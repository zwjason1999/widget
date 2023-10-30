#include "logger.h"

#include <ctime>
#include <string>
#include <cstdio>
#include <cstdarg>

namespace tinylog
{

void Logger::SetLogLevel(LogLevel level)
{
    logLevel_ = level;
}

void Logger::SetLogFile(std::string logFilePath)
{
    logFileStream_.open(logFilePath, std::ios::out | std::ios::trunc);
    if (!logFileStream_.is_open()) {
        printf("Failed to open file: %s\n", logFilePath.c_str());
        return;
    }
    logStream_ = &logFileStream_;
}

void Logger::Log(LogLevel level, const char* fileName, int lineNumber, const char* format, ...) {
    // 检查日志等级
    if (logLevel_ == LogLevel::CLOSE || level < logLevel_) {
        return;  
    }

    // 获取当前时间
    std::string currentTime = GetCurrentTime();

    // 构造日志头部信息
    std::string logLevelStr;
    switch (level) {
        case LogLevel::DEBUG:
            logLevelStr = "DEBUG";
            break;
        case LogLevel::INFO:
            logLevelStr = "INFO";
            break;
        case LogLevel::WARN:
            logLevelStr = "WARN";
            break;
        case LogLevel::ERROR:
            logLevelStr = "ERROR";
            break;
    }

    std::string logHeader = "[" + currentTime + "] - [" + logLevelStr + "] - [" + fileName + ": " + std::to_string(lineNumber) + "]: ";

    // 格式化日志消息
    va_list args;
    va_start(args, format);
    std::string logMessage = FormatString(format, args);
    va_end(args);

    // 输出日志
    std::unique_lock<std::mutex> lock(mutex_);
    *logStream_ << logHeader << logMessage;
    // TODO: 及时刷新缓冲区是否会影响性能呢
    logStream_->flush();
}

std::string Logger::GetCurrentTime() 
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    char buffer[20];
    localtime_r(&now, &timeinfo);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

std::string Logger::FormatString(const char* format, va_list args) 
{
    va_list argsCopy;
    va_copy(argsCopy, args); // 复制参数列表以安全地获取长度
    int length = vsnprintf(nullptr, 0, format, argsCopy);  // 不包含结尾 null 终止符
    va_end(argsCopy);

    if (length <= 0) {
        return "";  // 格式化失败
    }

    std::string result(length, '\0'); // 创建足够大的字符串
    vsnprintf(&result[0], length + 1, format, args); // +1 用于包含 null 终止符

    return result;
}

}   // namespace tinylog

void SetLogLevel(LogLevel level)
{
    tinylog::Logger::GetInstance().SetLogLevel(level);
}

void SetLogFile(std::string logFilePath)
{
    tinylog::Logger::GetInstance().SetLogFile(logFilePath);
}