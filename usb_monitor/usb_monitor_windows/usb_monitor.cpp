#include <Windows.h>
#include <SetupAPI.h>
#include <Dbt.h>
#include <usbiodef.h>
#include <initguid.h>
#include <SetupAPI.h>
#include <iostream>
#include <wchar.h>

//#include <devguid.h>
//#include <Cfgmgr32.h>
//#include <guiddef.h>

#pragma comment(lib, "SetupAPI.lib")

// 包括了4种USB设备: HUB, BILLBOARD, DEVICE, HOST_CONTROLLER
DEFINE_GUID(GUID_DEVINTERFACE_USB_HUB, 0xf18a0e88, 0xc30c, 0x11d0, 0x88, 0x15, 0x00, \
    0xa0, 0xc9, 0x06, 0xbe, 0xd8);

/*5e9adaef-f879-473f-b807-4e5ea77d1b1c*/
DEFINE_GUID(GUID_DEVINTERFACE_USB_BILLBOARD, 0x5e9adaef, 0xf879, 0x473f, 0xb8, 0x07, 0x4e, \
    0x5e, 0xa7, 0x7d, 0x1b, 0x1c);

/* A5DCBF10-6530-11D2-901F-00C04FB951ED */
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, \
    0xC0, 0x4F, 0xB9, 0x51, 0xED);

/* 3ABF6F2D-71C4-462a-8A92-1E6861E6AF27 */
DEFINE_GUID(GUID_DEVINTERFACE_USB_HOST_CONTROLLER, 0x3abf6f2d, 0x71c4, 0x462a, 0x8a, 0x92, 0x1e, \
    0x68, 0x61, 0xe6, 0xaf, 0x27);

const static GUID GUID_DEVINTERFACE_USB_LIST[] = {
    GUID_DEVINTERFACE_USB_HUB, 
    GUID_DEVINTERFACE_USB_BILLBOARD, 
    GUID_DEVINTERFACE_USB_DEVICE,
    GUID_DEVINTERFACE_USB_HOST_CONTROLLER
};

// 推荐的USB接口
// 用于检测USB设备是否从该接口插入
// 不同计算机需要通过插拔测试来得到
const wchar_t RECOMMENDED_USB_INTERFACE[] = L"PCIROOT(0)#PCI(1400)#USBROOT(0)#USB(20)";
const int RECOMMENDED_USB_INTERFACE_LENGTH = 39;

// 检查USB设备是否从推荐接口插入
bool isInsertIntoRecommendUSBInterface(const wchar_t* s1, const wchar_t* s2);

// 消息处理函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DEVICECHANGE)
    {
        PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;

        if (pHdr != nullptr)
        {
            switch (pHdr->dbch_devicetype)
            {
            case DBT_DEVTYP_DEVICEINTERFACE:
                // 获取设备结构体指针
                // 包含了有关设备的信息，包括设备类型、设备名称等
                PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
                if (pDevInf != nullptr)
                {
                    if (IsEqualGUID(pDevInf->dbcc_classguid, GUID_DEVINTERFACE_USB_DEVICE) ||
                        IsEqualGUID(pDevInf->dbcc_classguid, GUID_DEVINTERFACE_USB_HUB) ||
                        IsEqualGUID(pDevInf->dbcc_classguid, GUID_DEVINTERFACE_USB_BILLBOARD) ||
                        IsEqualGUID(pDevInf->dbcc_classguid, GUID_DEVINTERFACE_USB_HOST_CONTROLLER))
                    {
                        if (wParam == DBT_DEVICEARRIVAL)
                        {
                            std::cout << "----------------------------" << std::endl;
                            std::cout << "USB设备插入" << std::endl;

                            // 获取设备信息集合的句柄
                            // 具体是哪些设备与第一个参数和最后一个参数有关
                            // 在这里只获取刚刚插入的USB设备信息集合
                            HDEVINFO hDevInfo = SetupDiGetClassDevsW(
                                &pDevInf->dbcc_classguid, 
                                NULL, NULL, 
                                DIGCF_DEVICEINTERFACE | DIGCF_PRESENT
                            );
                            if (hDevInfo == NULL) 
                            {
                                std::cerr << "获取设备信息集合的句柄失败: " << GetLastError() << std::endl;
                                return -1;
                            }
                            // 获取每一个设备的详细信息
                            SP_DEVINFO_DATA devInfoData;
                            devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                            if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &devInfoData))
                            {
                                std::cerr << "获取设备的详细信息失败: " << GetLastError() << std::endl;
                                return -1;
                            }
                          
                            // 获取设备的设备树位置
                            wchar_t deviceLocationPaths[200];   // 缓冲区大小如果不足, 会出现莫名其妙的输出错误
                            
                            if (!SetupDiGetDeviceRegistryPropertyW(
                                hDevInfo,
                                &devInfoData,
                                SPDRP_LOCATION_PATHS,
                                NULL,
                                (PBYTE)&deviceLocationPaths,
                                sizeof(deviceLocationPaths),
                                NULL
                            ))
                            {
                                std::cerr << "获取设备的设备树位置信息失败: " << GetLastError() << std::endl;
                                return -1;
                            }
                            std::wcout << "设备树位置: " << deviceLocationPaths << std::endl;

                            // 检查是否从推荐USB接口插入
                            if (!isInsertIntoRecommendUSBInterface(RECOMMENDED_USB_INTERFACE, deviceLocationPaths))
                            {
                                // 警告信息
                                std::cout << "警告: USB设备从非推荐接口插入!!!" << std::endl;
                            }

                            // 销毁设备信息列表
                            SetupDiDestroyDeviceInfoList(hDevInfo);
                            std::cout << "----------------------------" << std::endl << std::endl;
                        }
                        else if (wParam == DBT_DEVICEREMOVECOMPLETE)
                        {
                            std::cout << "----------------------------" << std::endl;
                            std::cout << "USB设备拔出" << std::endl;
                            std::cout << "----------------------------" << std::endl << std::endl;
                        }
                    }
                }
                break;
            }
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int main()
{
    // 创建窗口类
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"USBWatcher";
    RegisterClass(&wc);

    // 创建窗口
    HWND hWnd = CreateWindow(
        wc.lpszClassName, nullptr, 
        0, 0, 0, 0, 0, 
        HWND_MESSAGE, 
        nullptr, nullptr, nullptr
    );

    // 注册USB设备插拔事件通知
    const int n = sizeof(GUID_DEVINTERFACE_USB_LIST) / sizeof(GUID);    // USB设备列表大小
    DEV_BROADCAST_DEVICEINTERFACE filters[n];
    HDEVNOTIFY hNotifys[n];
    for (int i = 0; i < n; ++i) 
    {
        filters[i] = {0};
        filters[i].dbcc_size = sizeof(filters[i]);
        filters[i].dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        filters[i].dbcc_classguid = GUID_DEVINTERFACE_USB_LIST[i];
        hNotifys[i] = RegisterDeviceNotification(
            hWnd, 
            &filters[i], 
            DEVICE_NOTIFY_WINDOW_HANDLE
        );
        if (hNotifys[i] == NULL)
        {
            std::cout << "RegisterDeviceNotification() error: " << GetLastError() << std::endl;
            return -1;
        }
    }

    // 监听设备变更消息
    std::cout << "开始监听..." << std::endl;
    std::cout << "设备位置的输出格式为: PCIROOT(x1)#PCI(x2)#USBROOT(x3)#USB(x4)#USB(x5)......" << std::endl;
    std::cout << "PCIROOT(x)   表示总线根节点编号为 x" << std::endl;
    std::cout << "PCI(x)       表示总线编号为 x" << std::endl;
    std::cout << "USBROOT(x)   表示USB根节点编号为 x" << std::endl;
    std::cout << "USB(x)...... 表示从USB根节点到当前位置的端口号路径" << std::endl << std::endl;

    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 注销USB设备插拔事件通知
    for (int i = 0; i < n; ++i)
    {   
        if (hNotifys[i]) 
        {
            UnregisterDeviceNotification(hNotifys[i]);
        }
    }

    return 0;
}

bool isInsertIntoRecommendUSBInterface(const wchar_t* s1, const wchar_t* s2)
{
    return wcsncmp(s1, s2, RECOMMENDED_USB_INTERFACE_LENGTH) == 0;
}