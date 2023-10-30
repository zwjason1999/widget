#include <cstring>
#include <cstdio>
#include "base.h"
#include "logger.h"
#include <libusb-1.0/libusb.h>

uint16_t SPECIFIED_HUB_VID = 0x1a40;
uint16_t SPECIFIED_HUB_PID = 0x0101;
char *SPECIFIED_HUB_TREE_PATH = new char[MAX_TREE_PATH_SIZE];

void Init()
{
    const libusb_version* lv = libusb_get_version();
    printf("major=%u, minor=%u, ,micro=%u, nano=%u\n", lv->major, lv->minor, lv->micro, lv->nano);
}

void Cleanup()
{
    if (NULL != SPECIFIED_HUB_TREE_PATH) {
        delete[] SPECIFIED_HUB_TREE_PATH;
    }
}

bool IsSpecifiedHub(uint16_t vid, uint16_t pid)
{
    return vid == SPECIFIED_HUB_VID && pid == SPECIFIED_HUB_PID;
}

/**
 * @brief 获取设备树路径
 *       格式: <bus_number>-<port1.port2.....>
 *       例如: 1-1.2.3
 * @param device [in] 设备指针
 * @param buffer [out] 用于存储设备树路径的缓存
 * @param buffer_size [in] 缓存容量
 */
void GetTreePath(libusb_device *device, char *buffer, uint32_t buffer_size)
{  
    // 获取一个设备的从根端口号到当前端口号的路径上的所有端口号
    uint8_t port_numbers[MAX_PORT_NUMBERS_SIZE];
    int len = libusb_get_port_numbers(device, port_numbers, MAX_PORT_NUMBERS_SIZE);
    if (len == LIBUSB_ERROR_OVERFLOW) {
        ERROR("libusb_get_port_numbers() error: %s\n", libusb_strerror((libusb_error)len));
        return;
    }
    // 拼接总线号和格式符-
    if (sprintf(buffer, "%d-", libusb_get_bus_number(device)) < 0) {
        ERROR("Buffer size is too small.\n");
        return;
    }
    // 拼接端口号
    for (int i = 0; i < len; ++i) {    
        if (sprintf(buffer, "%s%d.", buffer, port_numbers[i]) < 0) {
            ERROR("Buffer size is too small.\n");
            return;
        }
    }
    // 最后多了一个.符号, 改成结束符
    buffer[strlen(buffer) - 1] = '\0'; 
}

void GetDeviceInfo(libusb_device *device, DeviceInfo *device_info)
{
    // 获取设备树路径
    GetTreePath(device, device_info->tree_path, sizeof(device_info->tree_path));
    // 获取设备描述符
    int status;     // 用于保存各种返回值
    struct libusb_device_descriptor desc;
    status = libusb_get_device_descriptor(device, &desc);
    if (status != LIBUSB_SUCCESS) {
        ERROR("libusb_get_device_descriptor() error: %s\n", libusb_strerror((libusb_error)status));
    }
    // 获取VID和PID
    device_info->vid = desc.idVendor;
    device_info->pid = desc.idProduct;
    // 获取总线号和设备号
    device_info->bus_number = libusb_get_bus_number(device);
    device_info->device_number = libusb_get_device_address(device);
    // 按照系统格式, 总线号和设备号都不会超过3位数
    if (device_info->bus_number >= 1000 || device_info->device_number >= 1000) {
        ERROR("Invalid bus number: %u or device number: %u\n", device_info->bus_number, device_info->device_number);
        return;
    }
    
    // 补前导0固定长度为3位数
    char bus_number[4], device_number[4];
    if (sprintf(bus_number, "%03u", device_info->bus_number) < 0) {
        ERROR("Buffer size is too small.\n");
        return;
    }
    if (sprintf(device_number, "%03u", device_info->device_number) < 0) {
        ERROR("Buffer size is too small.\n");
        return;
    }

    // 构造命令
    char cmd[64];
    sprintf(cmd, "udevadm info --query=property --name=/dev/bus/usb/%s/%s", bus_number, device_number);
    FILE *output = popen(cmd, "r");
    if (output == NULL) {
        ERROR("Failed to run udevadm info.\n");
        return;
    }
    INFO("Succeed in running cmd: %s\n", cmd);

    // 按行读输出
    char line[256];
    while (NULL != fgets(line, sizeof(line), output)) {
        string tmp = line;
        size_t equal_pos = tmp.find('=');
        if (equal_pos != std::string::npos) {
            device_info->udevadm_info[tmp.substr(0, equal_pos)] = tmp.substr(equal_pos + 1);
        }
    }

    pclose(output);
}

void SetSpecifiedHubTreePath(const char *tree_path)
{
    size_t n = strlen(tree_path);
    if (n >= MAX_TREE_PATH_SIZE) {
        ERROR("Fail to set specified hub tree path\n");
        return;
    }
    strncpy(SPECIFIED_HUB_TREE_PATH, tree_path, n);
    SPECIFIED_HUB_TREE_PATH[n] = '\0';      
}

bool IsLegalTreePath(char *tree_path)
{
    size_t n = strlen(SPECIFIED_HUB_TREE_PATH);
    if (n == 0) {
        return false;
    }
    return strncmp(tree_path, SPECIFIED_HUB_TREE_PATH, n) == 0;
}


