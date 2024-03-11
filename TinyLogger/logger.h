#ifndef MYWEBSERVER_LOG_LOGGER_H_
#define MYWEBSERVER_LOG_LOGGER_H_

/**
 * 对外使用的日志类
 * 应该实现为单例模式: (1) 禁止拷贝复制; (2) 线程安全; (3) 实例初始化时机, 懒汉式和饿汉式
 * 1. 饿汉式: 程序运行时就初始化, 一般类内或类外手动创建单例对象
 * 2. 懒汉式: 双重 if + 互斥锁保证只实例化一次
 * 3. C++11 Mayers Singleton, 采用静态局部变量
*/

#include "../util/noncopyable.h"
#include "../sync/count_down_latch.h"
#include "log_writer.h"

#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include <cstdarg>

// 日志等级
enum class LogLevel
{
    CLOSE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class Logger : noncopyable
{
private:

    size_t flushInterval_;
    LogLevel level_;
    std::unique_ptr<LogWriter> writer_;
    bool running_;
    std::thread thread_; 
    CountdownLatch latch_;
    std::mutex mutex_;
    std::condition_variable cond_;

    size_t buffer_size_;
    std::shared_ptr<std::vector<char>> buffer1_;
    std::shared_ptr<std::vector<char>> buffer2_;
    std::shared_ptr<std::vector<char>> buffer3_;
    std::shared_ptr<std::vector<char>> buffer4_;
    std::vector<std::shared_ptr<std::vector<char>>> bucket1_;
    std::vector<std::shared_ptr<std::vector<char>>> bucket2_;
    
    Logger():
        running_(false),
        latch_(1)    
    {}

    ~Logger()
    {
        running_ = false;
        cond_.notify_all();     // 最后一次唤醒消费者线程, 将缓冲区内剩余的日志全部写入文件
        if (thread_.joinable()) {   // 等待线程消费者线程结束
            thread_.join();
        }
    }


    std::string GetCurrentTime();
    std::string FormatString(const char* format, va_list args);

    void AsyncLogging();

public:
    // 单例模式
    static Logger& Instance()
    {
        static Logger instance;
        return instance;
    }

    // 提供一个接口, 指定日志文件名, 设置日志等级, 设置缓冲区大小
    void Init(const std::string& filename, LogLevel level, size_t flushInterval = 3, size_t buffer_size = 2 * 1024);
    void Log(LogLevel level, const char* level_name, const char* filename, size_t line_number, const char* format, ...);
};


#define DEBUG(format, ...) Logger::Instance().Log(LogLevel::DEBUG, "DEBUG", __FILE__, __LINE__, format, ##__VA_ARGS__)
#define INFO(format, ...) Logger::Instance().Log(LogLevel::INFO, "INFO", __FILE__, __LINE__, format, ##__VA_ARGS__)
#define WARN(format, ...) Logger::Instance().Log(LogLevel::WARN, "WARN", __FILE__, __LINE__, format, ##__VA_ARGS__)
#define ERROR(format, ...) Logger::Instance().Log(LogLevel::ERROR, "ERROR", __FILE__, __LINE__, format, ##__VA_ARGS__)
#define FATAL(format, ...) Logger::Instance().Log(LogLevel::FATAL, "FATAL", __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif  // MYWEBSERVER_LOG_LOGGER_H_