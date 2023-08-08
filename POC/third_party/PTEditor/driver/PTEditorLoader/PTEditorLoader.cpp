#include <iostream>
#include <windows.h>
#pragma comment(lib,"advapi32.lib")

#define PTEDITOR_DEVICE_NAME L"PTEditorLink"
#define PTEDITOR_DEVICE_PATH L"\\\\.\\" PTEDITOR_DEVICE_NAME

#define DRIVER_NAME L"PTEdit.sys"
#define SERVICE_NAME L"PTEditor"

int main(int argc, char* argv[])
{
    SC_HANDLE hSCManager;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS ss;
    WCHAR driverPath[1024];
    int unload = 0;

    if (argc >= 2 && !strcmp(argv[1], "--unload")) unload = 1;

    HANDLE fd = CreateFile(DRIVER_NAME, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (fd == INVALID_HANDLE_VALUE) {
        printf("[-] Could not find driver PTEdit.sys in current directory\n");
        return 1;
    }
    else {
        CloseHandle(fd);
        GetFullPathName(DRIVER_NAME, sizeof(driverPath) / sizeof(driverPath[0]) - 1, driverPath, NULL);
    }
    printf("[+] Found driver: %ws\n", driverPath);

    // check if driver is loaded
    fd = CreateFile(PTEDITOR_DEVICE_PATH, GENERIC_ALL, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
    if (fd == INVALID_HANDLE_VALUE && unload) {
        printf("[-] Could not unload driver, driver is not loaded\n");
        return 1;
    }
    if (fd != INVALID_HANDLE_VALUE) {
        CloseHandle(fd);
        if (!unload) {
            printf("[+] PTEditor driver is already loaded. To unload the driver, run %s --unload\n", argv[0]);
            return 0;
        }
    }

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

    if (unload) {
        printf("[+] Unload PTEditor driver\n");
    }
    else {
        printf("[+] Load PTEditor driver\n");
    }

    if (hSCManager)
    {
        if (!unload) {
            printf("[+] Creating service\n");
        }
        else {
            printf("[+] Connecting to service\n");
        }

        hService = CreateService(hSCManager, SERVICE_NAME,
            L"PTEditor Driver",
            SERVICE_START | DELETE | SERVICE_STOP,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            driverPath,
            NULL, NULL, NULL, NULL, NULL);

        if (!hService)
        {
            printf("[+] Service not running, try to open service\n");
            hService = OpenService(hSCManager, SERVICE_NAME,
                SERVICE_START | DELETE | SERVICE_STOP);
        }

        if (hService)
        {
            if (!unload) {
                printf("[+] Starting service\n");
                StartService(hService, 0, NULL);
            }
            else {
                printf("[+] Stopping service\n");
                ControlService(hService, SERVICE_CONTROL_STOP, &ss);
                DeleteService(hService);
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCManager);
    }
    return 0;
}
