#include "file_writer.h"

#include <cstdio>

FileWriter::FileWriter(const std::string& filename):
    fp_(fopen(filename.c_str(), "a+"))
{
    // 设置缓存区
    // 替换 C 语言的默认缓冲行为: 
    // (1) 行缓存; (2) 块缓冲; (3) 无缓冲
    setbuffer(fp_, buffer_, sizeof(buffer_));
}

FileWriter::~FileWriter()
{
    fclose(fp_);
}

void FileWriter::Append(const char* logLine, size_t length)
{
    size_t offset = 0;
    while (offset < length) {
        size_t actual = this->Write(logLine + offset, length - offset);
        if (actual == 0 && ferror(fp_)) {
            fprintf(stderr, "FileWriter::Append() failed!\n");
            break;
        }
        offset += actual;
    }
}

void FileWriter::Flush()
{
    fflush(fp_);
}

size_t FileWriter::Write(const char* logLine, size_t length)
{
    // 为什么不用线程安全的 fwrite 
    // 而用 unlocked 不带文件锁的形式呢?
    // 因为在高一级实现 LogFile 中使用了互斥锁来保证线程安全
    // 其实就是赌 "互斥锁 + fwrite_unlocked" 比 “fwrite” 快
    return fwrite_unlocked(logLine, 1, length, fp_);    
}

