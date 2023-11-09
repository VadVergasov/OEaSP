#include <windows.h>
#include <tchar.h>
#include <thread>
#include <string>

HANDLE hMutex;
HWND hwnd;

std::thread thread1, thread2;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // Рисование в окне
            TextOut(hdc, 10, 10, _T("Mutex Synchronization Demo"), _tcslen(_T("Mutex Synchronization Demo")));
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            CloseHandle(hMutex);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void WorkerFunction(const wchar_t *threadName, int delay) {
    for (int i = 0; i < 5; ++i) {
        WaitForSingleObject(hMutex, INFINITE);

        SetWindowText(hwnd, threadName);

        ReleaseMutex(hMutex);

        Sleep(delay);
    }
    OutputDebugString((L"End: " + std::wstring(threadName) + L"\n").c_str());
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hMutex = CreateMutex(NULL, FALSE, NULL);
    if (hMutex == NULL) {
        MessageBox(NULL, _T("Mutex creation failed."), _T("Error"), MB_ICONERROR);
        return 1;
    }

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = _T("MutexSyncDemo");
    RegisterClass(&wc);

    hwnd = CreateWindow(_T("MutexSyncDemo"), _T("Lab6, OEaSP"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        400, 200, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, _T("Window creation failed."), _T("Error"), MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    thread1 = std::thread(WorkerFunction, L"Thread 1", 500);
    thread2 = std::thread(WorkerFunction, L"Thread 2", 700);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    thread1.join();
    thread2.join();

    return 0;
}
