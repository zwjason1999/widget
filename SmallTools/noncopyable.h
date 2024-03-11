#ifndef MYWEBSERVER_UTIL_noncopyable_H_
#define MYWEBSERVER_UTIL_noncopyable_H_

/**
 * 编写一个基类, 使得继承它的类都具有不可拷贝的属性
 * 即任何继承了该类的其他类, 其实例对象将不能被拷贝
 * 做法是删除拷贝构造函数和拷贝赋值运算符
*/

class noncopyable
{
private:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif
