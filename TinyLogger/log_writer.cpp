#include "log_writer.h"

LogWriter::LogWriter(const std::string& filename, size_t flushCycle):
    filename_(filename),
    flushCycle_(flushCycle),
    writeCount_(0),
    mutex_(),
    file_(new FileWriter(filename))
{}

void LogWriter::Append(const char* logLine, size_t length)
{
    std::unique_lock<std::mutex> locker(mutex_);
    this->AppendUnlocked(logLine, length);
}

void LogWriter::Flush()
{
    file_->Flush();
}

void LogWriter::AppendUnlocked(const char* logLine, size_t length)
{
    file_->Append(logLine, length);
    ++writeCount_;
    if (writeCount_ >= flushCycle_) {
        writeCount_ = 0;
        file_->Flush();
    }
}

