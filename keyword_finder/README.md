# 程序作用介绍
用户通过ssh连接到服务器上执行过的所有命令将被记录下来，存放在一个名为ssh的目录下。在ssh目录下都是`9`位数字`ID`标识的ssh会话目录，会话目录中分别有`tp-ssh-cmd.txt`文件和`tp-sshdeltime.dat`文件。

需求是根据指定的`ID`范围，在对应会话目录下的`tp-ssh-cmd.txt`文件中查找`input`关键字，在`tp-sshdeltime.dat`文件中查找`output`关键字。只要在任意一个文件中找到指定的关键字，就记录该会话的`ID`。

本程序提供`config.json`配置文件用于用户指定关键字。

![image-20221214200918475](https://img-bed-1304092357.cos.ap-guangzhou.myqcloud.com/keyword_finder_1.png)

# 环境与编译运行说明
1. 本程序在`ubuntu 18.04`上开发，`gcc 4.8.5`或以上；
2. 自行安装`jsoncpp`库，并在`Makefile`中指定`jsoncpp_path`；
3. 执行`make`即可生成可执行文件；
4. 结果打印到标准输出。
