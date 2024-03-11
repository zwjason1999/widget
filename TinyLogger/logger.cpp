#include "logger.h"

#include <assert.h>
#include <functional>
#include <cstring>

void Logger::Init(const std::string& filename, LogLevel level, size_t flushInterval, size_t buffer_size)
{   
    assert(filename.length() > 0);
    assert(flushInterval > 0);
    assert(buffer_size >= 128);
    
    running_ = true;
    flushInterval_ = flushInterval;
    level_ = level;
    writer_.reset(new LogWriter(filename));
    
    buffer_size_ = buffer_size;
    buffer1_ = std::make_shared<std::vector<char>>();
    buffer1_->reserve(buffer_size);

    buffer2_ = std::make_shared<std::vector<char>>();
    buffer2_->reserve(buffer_size);

    buffer3_ = std::make_shared<std::vector<char>>();
    buffer3_->reserve(buffer_size);

    buffer4_ = std::make_shared<std::vector<char>>();
    buffer4_->reserve(buffer_size);

    bucket1_.clear();
    bucket2_.clear();

    thread_ = std::thread(&Logger::AsyncLogging, this);     // 启动消费者线程
    latch_.Wait();  
}

void Logger::Log(LogLevel level, const char* level_name, const char* filename, size_t line_number, const char* format, ...)
{
    if (!running_ || level_ == LogLevel::CLOSE || level < level_) {
        return;  
    }

    /**
     * 构造消息头
     * 格式为: [时间] - [日志等级] - [文件名: 行号]:\n
    */
    std::string current_time = GetCurrentTime();
    std::string log_content = "[" + current_time + "] - [" + level_name + "] - [" + filename + ": " + std::to_string(line_number) + "]:\n";


    // 格式化日志内容
    va_list args;
    va_start(args, format);
    log_content += FormatString(format, args);
    log_content += '\n';
    va_end(args);


    std::unique_lock<std::mutex> locker(mutex_);
    if (buffer1_->capacity() - buffer1_->size() >= log_content.length()) {
        buffer1_->insert(buffer1_->end(), log_content.begin(), log_content.end());
    }
    else {
        bucket1_.push_back(std::move(buffer1_));
        if (buffer2_) {
            buffer1_ = std::move(buffer2_);
        }
        else {
            buffer1_ = std::make_shared<std::vector<char>>();
            buffer1_->reserve(buffer_size_);
            fprintf(stderr, "log too fast\n");
        }
        buffer1_->insert(buffer1_->end(), log_content.begin(), log_content.end());
        cond_.notify_one();
    }

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
    va_list args_copy;
    va_copy(args_copy, args); // 复制参数列表以安全地获取长度
    int length = vsnprintf(nullptr, 0, format, args_copy);  // 不包含结尾 null 终止符
    va_end(args_copy);

    if (length <= 0) {
        return "";  // 格式化失败
    }

    std::string result(length, '\0'); // 创建足够大的字符串
    vsnprintf(&result[0], length + 1, format, args); // +1 用于包含 null 终止符

    return result;
}

void Logger::AsyncLogging()
{
    latch_.Countdown();
    while (running_) {
        {
            assert(buffer3_ && buffer3_->size() == 0);
            assert(buffer4_ && buffer4_->size() == 0);
            assert(bucket2_.empty());

            std::unique_lock<std::mutex> locker(mutex_);
            if (bucket1_.empty()) {
                cond_.wait_for(locker, std::chrono::seconds(flushInterval_));
            }
            bucket1_.push_back(std::move(buffer1_));
            buffer1_ = std::move(buffer3_);
            bucket2_.swap(bucket1_);
            if (!buffer2_) {
                buffer2_ = std::move(buffer4_);
            }
        }

        assert(!bucket2_.empty());

        // 短时间内产生大量日志, 程序可能发生异常
        // 一个 buffer 大小 2M
        // 8个就是 16M
        // 瞬间产生大量日志
        if (bucket2_.size() > 8) {
            const char* warning = "Program Exception: Too Many Logs in a Short Time. Discarding redundant Logs";
            writer_->Append(warning, strlen(warning));
            writer_->Flush();
            bucket2_.erase(bucket2_.begin() + 2, bucket2_.end());
            fprintf(stderr, "%s\n", warning);
        }

        // 写入文件
        for (const auto& buffer : bucket2_) {
            writer_->Append(buffer->data(), buffer->size());
        }

        // 丢弃因短时间内产生大量日志而重新分配的缓冲区
        // 留下常用的那两个
        if (bucket2_.size() > 2) {
            bucket2_.resize(2);
        }

        if (!buffer3_) {
            assert(!bucket2_.empty());
            buffer3_ = std::move(bucket2_.back());
            bucket2_.pop_back();
            buffer3_->clear();
        }

        if (!buffer4_) {
            assert(!bucket2_.empty());
            buffer4_ = std::move(bucket2_.back());
            bucket2_.pop_back();
            buffer4_->clear();
        }

        writer_->Flush();
        bucket2_.clear();
    }
    writer_->Flush();
}