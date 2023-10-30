# 环境要求
gcc >= 4.8.5(支持C++11)

# 安装依赖
1. libusb 库安装：
    1. Ubuntu 等发行版使用命令`sudo apt install libusb-1.0-0-dev`安装；
    2. CentOS 等发行版使用`yum`命令安装；
    3. 源码方式安装 [libusb github](https://github.com/libusb/libusb)。
2. 使用命令`pkg-config --modversion libusb-1.0`查看版本确认是否安装成功。

# 注册自家USB设备

```cpp
// src/base.cpp
uint16_t SPECIFIED_HUB_VID = 0x1a40;
uint16_t SPECIFIED_HUB_PID = 0x0101;
```

修改 VID 和 PID 然后重新编译即可。

# 编译运行

1. 如果 libusb 安装在默认路径下(即按照上面的方法安装), 直接`sudo make`生成可执行文件；
2. 以**root权限/管理员身份**运行即可`sudo ./usb_monitor`。




