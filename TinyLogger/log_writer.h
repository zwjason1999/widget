#ifndef MYWEBSERVER_LOG_LOGWRITER_H_
#define MYWEBSERVER_LOG_LOGWRITER_H_

/**
 * 使用互斥锁进一步封装 LogWriter 类
 * 提供线程安全的写文件操作接口
*/

#include "file_writer.h"
#include "../util/noncopyable.h"

#include <memory>
#include <string>
#include <mutex>


class LogWriter : noncopyable
{
private:
    const std::string filename_;
    const size_t flushCycle_;
    size_t writeCount_;
    std::mutex mutex_;
    std::unique_ptr<FileWriter> file_;

    void AppendUnlocked(const char* logLine, size_t length);

public:
    LogWriter(const std::string& filename, size_t flushCycle = 1024);
    ~LogWriter() = default;

    void Append(const char* logLine, size_t length);
    void Flush();
    // bool RollFile();
};

#endif
