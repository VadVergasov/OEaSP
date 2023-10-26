#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>

HINSTANCE hInstance;
HWND hMainWindow;

std::wstring GetErrorMessage(DWORD errorCode) {
    LPVOID errorMsg = nullptr;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errorCode,
        0,
        (LPWSTR)&errorMsg,
        0,
        NULL
    );
    if (errorMsg != NULL) {
        std::wstring message(static_cast<LPCWSTR>(errorMsg));
        LocalFree(errorMsg);
        return message;
    }
    return L"Unknown Error";
}

void LogEvent(const std::wstring &message, WORD eventType) {
    HANDLE hEventLog = RegisterEventSource(NULL, L"TTLChanger");
    if (hEventLog != NULL) {
        const wchar_t *messageStrings[1] = { message.c_str() };
        ReportEvent(hEventLog, eventType, 0, 0, NULL, 1, 0, messageStrings, NULL);
        DeregisterEventSource(hEventLog);
    }
}

bool GetRegistryValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, DWORD &result) {
    HKEY hSubKey;
    if (RegOpenKeyEx(hKey, subKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
        DWORD valueType;
        DWORD dataSize = sizeof(result);
        if (RegQueryValueEx(hSubKey, valueName, NULL, &valueType, (LPBYTE)&result, &dataSize) == ERROR_SUCCESS) {
            RegCloseKey(hSubKey);
            return true;
        }
        RegCloseKey(hSubKey);
    }
    return false;
}

void CreateValue(HWND hWnd, HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, DWORD value) {
    int result = MessageBox(hWnd, L"Значение DefaultTTL не равно 65. Хотите создать его?", L"Результат проверки реестра", MB_ICONQUESTION | MB_YESNO);
    if (result == IDYES) {
        HKEY hKey;
        auto result_code = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_WRITE, &hKey);
        if (result_code == ERROR_SUCCESS) {
            DWORD newValue = 65;
            result_code = RegSetValueEx(hKey, valueName, 0, REG_DWORD, (const BYTE *)&newValue, sizeof(newValue));
            if (result_code == ERROR_SUCCESS) {
                LogEvent(L"Настройка создана!", EVENTLOG_SUCCESS);
                MessageBox(hWnd, L"Значение DefaultTTL было успешно создано с значением 65.", L"Результат создания значения", MB_ICONINFORMATION);
            } else {
                LogEvent(L"Ошибка при создании значения DefaultTTL: " + GetErrorMessage(result_code), EVENTLOG_ERROR_TYPE);
                MessageBox(hWnd, (L"Ошибка при создании значения DefaultTTL: " + GetErrorMessage(result_code)).c_str(), L"Результат создания значения", MB_ICONERROR);
            }
            RegCloseKey(hKey);
        } else {
            LogEvent(L"Ошибка при создани: " + GetErrorMessage(result_code), EVENTLOG_ERROR_TYPE);
            MessageBox(hWnd, GetErrorMessage(result_code).c_str(), L"Ошибка", MB_ICONERROR);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LPCWSTR registryPath = L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
    LPCWSTR valueName = L"DefaultTTL";
    DWORD value;
    switch (message) {
        case WM_CREATE:
            if (GetRegistryValue(HKEY_LOCAL_MACHINE, registryPath, valueName, value)) {
                if (value == 65) {
                    LogEvent(L"Настройка уже существует", EVENTLOG_INFORMATION_TYPE);
                    MessageBox(hWnd, L"Значение 65 для DefaultTTL уже существует в реестре.", L"Результат проверки реестра", MB_ICONINFORMATION);
                } else {
                    CreateValue(hWnd, HKEY_LOCAL_MACHINE, registryPath, valueName, 65);
                }
            } else {
                CreateValue(hWnd, HKEY_LOCAL_MACHINE, registryPath, valueName, 65);
            }
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RegistryCheckApp"), NULL };
    RegisterClassEx(&wc);
    hInstance = hInst;
    hMainWindow = CreateWindow(wc.lpszClassName, L"Проверка реестра", WS_OVERLAPPEDWINDOW, 100, 100, 400, 200, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(hMainWindow, nCmdShow);
    UpdateWindow(hMainWindow);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
