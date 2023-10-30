#include <iostream>
#include <cstdlib>
#include <cstring>
#include <libusb-1.0/libusb.h>
 
#include "monitor.h"
#include "logger.h"
#include "base.h"
#include "control.h"

// 设备插入时的回调函数
int LIBUSB_CALL device_arrived_cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data);
// 设备拔出时的回调函数
int LIBUSB_CALL device_left_cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data);

void MonitorHotplug()
{
    libusb_context *ctx;    // 代表一个libusb会话的数据结构
    int status;             // 状态码, 用于接收各个函数调用的返回值
    // 初始化libusb库
    if ((status = libusb_init(&ctx)) < 0) {
        ERROR("libusb_init() error: %s\n", libusb_strerror((libusb_error)status));
        exit(-1);
    }
    // 设置DEBUG的等级
    // 输出更多错误信息以便修正程序
    // libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);

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
        ERROR("libusb_hotplug_register_callback() error: %s\n", libusb_strerror((libusb_error)status));
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
        ERROR("libusb_hotplug_register_callback() error: %s\n", libusb_strerror((libusb_error)status));
        exit(-1);
    }

    // 轮询事件
    printf("Start listening...\n");
    printf("Format: <bus>-<port1.port2......>:<device>\n\n");
    while (1) {
        status = libusb_handle_events_completed(ctx, NULL);
        if (status != 0) {
            ERROR("libusb_handle_events_completed() error: %s\n", libusb_strerror((libusb_error)status));
            break;
        }
    }
    
    // 回收回调函数的内存 
    libusb_hotplug_deregister_callback(ctx, device_arrived_handle);
    libusb_hotplug_deregister_callback(ctx, device_left_handle);
    // 关闭libusb库
    libusb_exit(ctx);
}


int LIBUSB_CALL device_arrived_cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
{
    printf("------------------------------------------------------------\n");
    // 获取设备信息
    DeviceInfo devic_info;
    GetDeviceInfo(device, &devic_info);
    if (IsSpecifiedHub(devic_info.vid, devic_info.pid)) {
        // 放行自家hub
        // 记录自家hub的设备树路径
        SetSpecifiedHubTreePath(devic_info.tree_path);
        printf("[Allowed] Specified hub insertion.\n");
    } else if (IsLegalTreePath(devic_info.tree_path)) {
        // 从自家hub插入的USB设备
        printf("[Allowed] USB device inserted from specified hub.\n");
    } else {
        // 阻止其他USB设备
        RemoveAuthorization(devic_info.tree_path);
        printf("[Blocked] Unknown USB device insertion.\n");
    }
    // 显示设备信息
    devic_info.Display();
    printf("------------------------------------------------------------\n\n");
    return 0;   // 必要的返回语句, 返回0表示没有出错, 还可以继续被调用
}

int LIBUSB_CALL device_left_cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
{
    printf("------------------------------\n");
    printf("USB device removal.\n");

    struct libusb_device_descriptor desc;
    int status = libusb_get_device_descriptor(device, &desc);
    if (status != LIBUSB_SUCCESS) {
        ERROR("libusb_get_device_descriptor() error: %s\n", libusb_strerror((libusb_error)status));
    }
    // 自家hub移除, 清空其设备树路径
    if (IsSpecifiedHub(desc.idVendor, desc.idProduct)) {
        SetSpecifiedHubTreePath("");
        printf("[NOTICE] Specified hub removal.\n");
    }
    printf("------------------------------\n\n");
    return 0;
}

