#ifndef _BASE_H_
#define _BASE_H_

#include <libusb-1.0/libusb.h>
#include <map>

// 当下技术端口号路径最大深度为 7
// 用 8 企图内存对齐
#define MAX_PORT_NUMBERS_SIZE 8

// MAX_TREE_PATH_SIZE的计算方法: 
// bus_number(1), port_number(7), 格式符.(6), 格式符-(1), 结束符\0(1)
// 翻一倍多 16 位给设备号, 虽然实际上要不了这么多
#define MAX_TREE_PATH_SIZE 32

using std::string;
using std::map;

extern uint16_t SPECIFIED_HUB_VID;
extern uint16_t SPECIFIED_HUB_PID;
extern char *SPECIFIED_HUB_TREE_PATH;


struct DeviceInfo
{
    uint16_t vid;
    uint16_t pid;

    uint8_t bus_number;
    uint8_t device_number;
    
    char tree_path[MAX_TREE_PATH_SIZE];

    // 存储udevadm info的输出
    map<string, string> udevadm_info;
    // 一个例子如下
    /*    
    BUSNUM=001
    DEVNAME=/dev/bus/usb/001/002
    DEVNUM=002
    DEVPATH=/devices/pci0000:00/0000:00:1d.0/usb1/1-1
    DEVTYPE=usb_device
    DRIVER=usb
    ID_BUS=usb
    ID_FOR_SEAT=usb-pci-0000_00_1d_0-usb-0_1
    ID_MODEL=07e6
    ID_MODEL_ENC=07e6
    ID_MODEL_ID=07e6
    ID_PATH=pci-0000:00:1d.0-usb-0:1
    ID_PATH_TAG=pci-0000_00_1d_0-usb-0_1
    ID_REVISION=0017
    ID_SERIAL=8087_07e6
    ID_USB_INTERFACES=:090000:
    ID_VENDOR=8087
    ID_VENDOR_ENC=8087
    ID_VENDOR_FROM_DATABASE=Intel Corp.
    ID_VENDOR_ID=8087
    MAJOR=189
    MINOR=1
    PRODUCT=8087/7e6/17
    SUBSYSTEM=usb
    TAGS=:seat:
    TYPE=9/0/1
    USEC_INITIALIZED=647717
    */

    DeviceInfo(): vid(0), pid(0), bus_number(0), device_number(0) {}

    void Display()
    {   
        printf("\t**********Device Information**********\n");
        printf("\tFull path: %s\n", tree_path);
        printf("\tVendor ID: %04x\n", vid);
        printf("\tProduct ID: %04x\n", pid);
        for (auto it = udevadm_info.begin(); it != udevadm_info.end(); ++it) {
            printf("\t%s: %s", it->first.c_str(), it->second.c_str());
        }
        printf("\t******End of Device Information*******\n");
    }
};



/**
 * @brief 一些初始化工作
 * 
 */
void Init();

/**
 * @brief 一些收尾工作
 * 
 */
void Cleanup();


/**
 * @brief 验证是否为自家HUB
 * 
 * @param vid 
 * @param pid 
 * @return bool
 */
bool IsSpecifiedHub(uint16_t vid, uint16_t pid);

/**
 * @brief 获取设备详细信息
 * 
 * @param device [in] 
 * @param device_info [out]
 */
void GetDeviceInfo(libusb_device *device, DeviceInfo *device_info);

/**
 * @brief Set the Hub Tree Path object
 * 
 * @param tree_path 
 */
void SetSpecifiedHubTreePath(const char *tree_path);

/**
 * @brief 判断一个设备树路径是否合法
 * 
 * @param tree_path 设备树路径
 * @return bool
 */
bool IsLegalTreePath(char *tree_path);




#endif