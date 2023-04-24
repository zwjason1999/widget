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

// ������4��USB�豸: HUB, BILLBOARD, DEVICE, HOST_CONTROLLER
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

// �Ƽ���USB�ӿ�
// ���ڼ��USB�豸�Ƿ�Ӹýӿڲ���
// ��ͬ�������Ҫͨ����β������õ�
const wchar_t RECOMMENDED_USB_INTERFACE[] = L"PCIROOT(0)#PCI(1400)#USBROOT(0)#USB(20)";
const int RECOMMENDED_USB_INTERFACE_LENGTH = 39;

// ���USB�豸�Ƿ���Ƽ��ӿڲ���
bool isInsertIntoRecommendUSBInterface(const wchar_t* s1, const wchar_t* s2);

// ��Ϣ������
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
                // ��ȡ�豸�ṹ��ָ��
                // �������й��豸����Ϣ�������豸���͡��豸���Ƶ�
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
                            std::cout << "USB�豸����" << std::endl;

                            // ��ȡ�豸��Ϣ���ϵľ��
                            // ��������Щ�豸���һ�����������һ�������й�
                            // ������ֻ��ȡ�ող����USB�豸��Ϣ����
                            HDEVINFO hDevInfo = SetupDiGetClassDevsW(
                                &pDevInf->dbcc_classguid, 
                                NULL, NULL, 
                                DIGCF_DEVICEINTERFACE | DIGCF_PRESENT
                            );
                            if (hDevInfo == NULL) 
                            {
                                std::cerr << "��ȡ�豸��Ϣ���ϵľ��ʧ��: " << GetLastError() << std::endl;
                                return -1;
                            }
                            // ��ȡÿһ���豸����ϸ��Ϣ
                            SP_DEVINFO_DATA devInfoData;
                            devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                            if (!SetupDiEnumDeviceInfo(hDevInfo, 0, &devInfoData))
                            {
                                std::cerr << "��ȡ�豸����ϸ��Ϣʧ��: " << GetLastError() << std::endl;
                                return -1;
                            }
                          
                            // ��ȡ�豸���豸��λ��
                            wchar_t deviceLocationPaths[200];   // ��������С�������, �����Ī��������������
                            
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
                                std::cerr << "��ȡ�豸���豸��λ����Ϣʧ��: " << GetLastError() << std::endl;
                                return -1;
                            }
                            std::wcout << "�豸��λ��: " << deviceLocationPaths << std::endl;

                            // ����Ƿ���Ƽ�USB�ӿڲ���
                            if (!isInsertIntoRecommendUSBInterface(RECOMMENDED_USB_INTERFACE, deviceLocationPaths))
                            {
                                // ������Ϣ
                                std::cout << "����: USB�豸�ӷ��Ƽ��ӿڲ���!!!" << std::endl;
                            }

                            // �����豸��Ϣ�б�
                            SetupDiDestroyDeviceInfoList(hDevInfo);
                            std::cout << "----------------------------" << std::endl << std::endl;
                        }
                        else if (wParam == DBT_DEVICEREMOVECOMPLETE)
                        {
                            std::cout << "----------------------------" << std::endl;
                            std::cout << "USB�豸�γ�" << std::endl;
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
    // ����������
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"USBWatcher";
    RegisterClass(&wc);

    // ��������
    HWND hWnd = CreateWindow(
        wc.lpszClassName, nullptr, 
        0, 0, 0, 0, 0, 
        HWND_MESSAGE, 
        nullptr, nullptr, nullptr
    );

    // ע��USB�豸����¼�֪ͨ
    const int n = sizeof(GUID_DEVINTERFACE_USB_LIST) / sizeof(GUID);    // USB�豸�б��С
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

    // �����豸�����Ϣ
    std::cout << "��ʼ����..." << std::endl;
    std::cout << "�豸λ�õ������ʽΪ: PCIROOT(x1)#PCI(x2)#USBROOT(x3)#USB(x4)#USB(x5)......" << std::endl;
    std::cout << "PCIROOT(x)   ��ʾ���߸��ڵ���Ϊ x" << std::endl;
    std::cout << "PCI(x)       ��ʾ���߱��Ϊ x" << std::endl;
    std::cout << "USBROOT(x)   ��ʾUSB���ڵ���Ϊ x" << std::endl;
    std::cout << "USB(x)...... ��ʾ��USB���ڵ㵽��ǰλ�õĶ˿ں�·��" << std::endl << std::endl;

    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // ע��USB�豸����¼�֪ͨ
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