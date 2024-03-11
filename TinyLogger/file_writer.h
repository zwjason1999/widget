#ifndef MYWEBSERVER_LOG_FILEWRITER_H_
#define MYWEBSERVER_LOG_FILEWRITER_H_

/**
 * 封装 C 语言 FILE 对象
 * 底层写文件接口
*/

#include "../util/noncopyable.h"

#include <string>


class FileWriter : noncopyable
{
private:
    size_t Write(const char* logLine, size_t length);
    FILE* fp_;
    char buffer_[64 * 1024];    // @todo: param 可配置参数之一

public:
    explicit FileWriter(const std::string& filename);
    ~FileWriter();
    //
    void Append(const char* logLine, const size_t length);
    void Flush();
};

#endif  // MYWEBSERVER_LOG_FILEWRITER_H_
