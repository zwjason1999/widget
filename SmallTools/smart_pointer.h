#include <iostream>
#include <mutex>


template <typename T>
class MySharedPtr 
{
private:
    T* ptr_;
    size_t* count_;
    std::mutex* mutex_;

public:
    MySharedPtr(): ptr_(nullptr), count_(new size_t(0)), mutex_(new std::mutex()) 
    {
        cout << "default ctor" << endl;
    }

    MySharedPtr(T* ptr): ptr_(ptr), count_(new size_t(1)), mutex_(new std::mutex()) 
    {
        cout << "specified ctor" << endl;
    }

    MySharedPtr(const MySharedPtr<T>& rhs)
    {
        // 拷贝构造函数无需证同测试
        // 既然正在构造对象, 对象都还没构造出来, 怎么可能被拷贝呢?
        this->ptr_ = rhs.ptr_;
        this->count_ = rhs.count_;
        this->mutex_ = rhs.mutex_;
        this->AddCount();

        cout << "copy ctor" << endl;
    }

    MySharedPtr<T>& operator=(const MySharedPtr<T>& rhs)
    {   
        // 拷贝赋值运算符需要进行需证同测试
        if (this != &rhs) {
            // 当前对象要被覆盖了, 要妥善处理它所管理的资源
            this->ReduceCount();
            // 善后完毕, 管理新的资源
            this->ptr_ = rhs.ptr_;
            this->count_ = rhs.count_;
            this->mutex_ = rhs.mutex_;
            this->AddCount();
        }
        cout << "copy operator" << endl;
        return *this;
    }

    MySharedPtr(MySharedPtr<T>&& rhs):
        ptr_(rhs.ptr_), count_(rhs.count_), mutex_(rhs.mutex_)
    {
        // 接管资源后, 被接管对象就不需要管了
        // 引用计数恰好不发生改变, 只是换个对象管理而已
        rhs.ptr_ = nullptr;
        rhs.count_ = nullptr;
        rhs.mutex_ = nullptr;

        cout << "move ctor" << endl;
    }

    MySharedPtr<T>& operator=(MySharedPtr<T>&& rhs)
    {
        if (this != &rhs) {
            ptr_ = rhs.ptr_;
            count_ = rhs.count_;
            mutex_ = rhs.mutex_;
            rhs.ptr_ = nullptr;
            rhs.count_ = nullptr;
            rhs.mutex_ = nullptr;
        }
        return *this;

        cout << "move operator" << endl;
    }

    ~MySharedPtr()
    {
        // 一个智能指针对象被析构, 就少了一个资源引用者
        ReduceCount();
        cout << "dtor" << endl;
    }

    T& operator*() const
    {
        return *ptr_; 
    }

    T* operator->() const
    {
        return ptr_;
    }

    size_t GetCount() const
    {
        std::unique_lock<std::mutex> locker(*mutex_);
        return *count_;
    }

    T* Get() const
    {
        return ptr_;
    }

private:
    void AddCount()
    {
        std::unique_lock<std::mutex> locker(*mutex_);
        ++*count_;
    }

    void ReduceCount()
    {   
        // 由于析构函数中调用了该函数
        // 为了防止被移动过的对象析构时访问空指针
        // 这里需要先判断一下资源是否已经被释放
        if (ptr_ == nullptr) {
            return;
        }

        std::unique_lock<std::mutex> locker(*mutex_);
        --*count_;
        if (*count_ == 0) {
            delete ptr_;
            delete count_;
            mutex_->~mutex();
            cout << "release" << endl;
        }
    }
};