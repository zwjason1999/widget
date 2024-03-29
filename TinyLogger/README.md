# 仿muduo的C风格日志系统

设计总共分 3 层：由上而下分别是 `logger.h -> log_writer.h -> file_writer.h` 

1. 最底层 `file_writer.h` 编写 FileWriter 类封装 C 语言 `FILE*` 文件对象，对上层提供写入文件接口。
    - 内部实现通过 `setbuffer()` 更改了内部缓冲区大小；封装 C 语言 `fwrite_unlocked()` 接口（该接口内部没有使用锁来保证写入串行化，而 `fwrite` 用了）。
2. 接着 `log_writer.h` 编写 `LogWriter` 类对 `FileWriter` 进一步封装，主要是使用互斥锁保证多线程安全，让日志顺序不发生错乱。
    - 日志完整性是在这一层使用互斥锁保证的，调用写入接口时使用互斥锁确保完整写入后再释放锁。
3. 最上层 `logger.h` 继续封装 `LogWriter` 对象，同时采用多重缓存、多线程等技术，基于 `<cstdarg>` 头文件实现 C 风格的日志写入接口
    - `Logger` 类 Mayers' Singleton 单例模式，即采用静态局部变量构造实例，属于懒汉式，但是线程安全（静态局部变量保证只会初始化一次）。 

最后通过宏定义对外提供分级日志接口。