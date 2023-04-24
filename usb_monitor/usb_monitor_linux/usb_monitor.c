#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

// 当下技术端口号路径最大深度为7
#define MAX_PORT_NUMBERS_LENGTH 7

// MAX_BUS_PORT_NUMBERS_LENGTH的计算方法: 
// bus_number(1), port_number(7), 格式符.(6), 格式符-(1), 结束符\0(1)
// 翻一倍多 16 位给设备号, 虽然实际上要不了这么多
#define MAX_BUS_PORT_NUMBERS_LENGTH 32

// 推荐的USB接口
// 需要手动插拔USB设备进行测试确定
// 测试使用命令 lsusb -t 和 dmesg | grep usb
const char *RECOMMENDED_USB_INTERFACE = "1-1";
const int RECOMMENDED_USB_INTERFACE_LENGTH = 3;

// 设备插入时的回调函数
int LIBUSB_CALL device_arrived_cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data);
// 设备拔出时的回调函数
int LIBUSB_CALL device_left_cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data);

int main()
{
    libusb_context *ctx;    // 代表一个libusb会话的数据结构
    int status;             // 状态码, 用于接收各个函数调用的返回值
    // 初始化libusb库
    if ((status = libusb_init(&ctx)) < 0) {
        printf("libusb_init() error: %s\n", libusb_strerror(status));
        exit(-1);
    }
    // 设置DEBUG的等级
    // 输出更多错误信息以便修正程序
    libusb_set_option(ctx, LIBUSB_LOG_LEVEL_DEBUG);

    // 注册设备插入的回调函数
    libusb_hotplug_callback_handle device_arrived_handle;
    status = libusb_hotplug_register_callback(
        ctx,
        LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
        LIBUSB_HOTPLUG_NO_FLAGS,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        device_arrived_cb,
        NULL,
        &device_arrived_handle
    );
    if (status != LIBUSB_SUCCESS) {
        printf("libusb_hotplug_register_callback() error: %s\n", libusb_strerror(status));
        exit(-1);
    }

    // 注册设备拔出的回调函数
    libusb_hotplug_callback_handle device_left_handle;
    status = libusb_hotplug_register_callback(
        ctx,
        LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
        LIBUSB_HOTPLUG_NO_FLAGS,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        device_left_cb,
        NULL,
        &device_left_handle
    );
    if (status != LIBUSB_SUCCESS) {
        printf("libusb_hotplug_register_callback() error: %s\n", libusb_strerror(status));
        exit(-1);
    }

    // 轮询事件
    printf("开始监听...\n");
    printf("输出格式: <主线号>-<端口号.端口号......>:<设备号>\n\n");
    while (1) {
        status = libusb_handle_events_completed(ctx, NULL);
        if (status != 0) {
            printf("libusb_handle_events_completed() error: %s\n", libusb_strerror(status));
            break;
        }
    }
    
    // 回收回调函数的内存 
    libusb_hotplug_deregister_callback(ctx, device_arrived_handle);
    libusb_hotplug_deregister_callback(ctx, device_left_handle);
    // 关闭libusb库
    libusb_exit(ctx);
    return 0;
}

int LIBUSB_CALL device_arrived_cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
{
    // printf("设备地址: %p\n", device);
    int bus_number = libusb_get_bus_number(device);
    int device_number = libusb_get_device_address(device);
    
    // 获取一个设备的从根端口号到当前端口号的路径上的所有端口号
    uint8_t port_numbers[MAX_PORT_NUMBERS_LENGTH];
    int real_port_numbers_length = 
        libusb_get_port_numbers(device, port_numbers, MAX_PORT_NUMBERS_LENGTH);

    // 格式化设备的全路径
    // 格式: <bus_number>-<port1.port2.....>:<device_number>
    // 例如: 1-1.2.3:15
    char str_bus_port_device_numbers[MAX_BUS_PORT_NUMBERS_LENGTH];
    sprintf(str_bus_port_device_numbers, "%d-", bus_number);   // 拼接bus_number和格式符-
    for (int i = 0; i < real_port_numbers_length; ++i) {    // 拼接端口号
        sprintf(str_bus_port_device_numbers, "%s%d.", str_bus_port_device_numbers, port_numbers[i]);
    }
    str_bus_port_device_numbers[strlen(str_bus_port_device_numbers) - 1] = '\0';  // 最后多了一个.
                                                                                    //改成结束符
    sprintf(str_bus_port_device_numbers, "%s:%d", str_bus_port_device_numbers, device_number);  // 拼接设备号                                                                
    
    printf("------------------------------\n");
    printf("USB设备插入\n");
    printf("其完整路径: %s\n", str_bus_port_device_numbers);
    // 检查该USB设备是否从推荐USB接口插入
    // 以推荐USB接口 RECOMMENDED_USB_INTERFACE = "1-1" 为例
    // 任何从该USB接口插入的USB设备的 str_bus_port_device_numbers 结果类似 "1-1.*"
    // 因此比较两者前缀即可
    if (strncmp(RECOMMENDED_USB_INTERFACE, str_bus_port_device_numbers, RECOMMENDED_USB_INTERFACE_LENGTH) != 0) {
        printf("警告: USB设备从非推荐接口插入!!!");
        printf("推荐的USB端口: %s\n", RECOMMENDED_USB_INTERFACE);
    }
    printf("------------------------------\n\n");
    
    return 0;   // 必要的返回语句, 返回0表示没有出错, 还可以继续被调用
}

int LIBUSB_CALL device_left_cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
{
    printf("------------------------------\n");
    printf("USB设备拔出\n");
    printf("------------------------------\n\n");
    return 0;
}

