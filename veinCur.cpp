#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <thread>
#include <mutex>
using namespace Gdiplus;
using namespace std;
#pragma comment (lib,"Gdiplus.lib")

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

Gdiplus::Graphics* graphics;
ULONG_PTR gdiplusToken;
HWND hwnd;
mutex mtx;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
void CreateFullScreenTransparentWindow(const wchar_t* window_name, const wchar_t* class_name);
int g_hotkeyId = 1;

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{

    HANDLE hMutex = CreateMutex(NULL, FALSE, L"cursorAnim");
    if (hMutex == NULL) {
        return 1;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(nullptr, L"已运行", L"错误", MB_OK);
        CloseHandle(hMutex);
        return 1;
    }

    CreateFullScreenTransparentWindow(L"鼠标特效", L"cursor_anim");
    HHOOK hook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);


    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Sleep(1);
    }
    CloseHandle(hMutex);
    return 0;
}

void CreateFullScreenTransparentWindow(const wchar_t* window_name, const wchar_t* class_name)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = class_name;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(255, 0, 0));
    if (!RegisterClass(&wc))
    {
        MessageBox(nullptr, L"窗口类注册失败", L"错误", MB_OK);
        return;
    }

    hwnd = CreateWindowExW(WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, class_name, window_name, WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    if (hwnd == nullptr)
    {
        MessageBox(nullptr, L"窗口创建失败", L"错误", MB_OK);
        return;
    }

    SetLayeredWindowAttributes(hwnd, RGB(255, 0, 0), 100, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        graphics = new Gdiplus::Graphics(hwnd);
        break;
    }
    case WM_TIMER:
        break;
    case WM_DESTROY:
    {
        MessageBox(nullptr, L"关闭", L"关闭", MB_OK);
        delete graphics;
        UnregisterHotKey(hwnd, g_hotkeyId);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }
    return 0;
}

void vine(int x1, int y1, float anchor, float turn, int speed, int cost, Color color, int count, int clone)
{
    if (speed <= 0)
    {
        Sleep(300);
        return;
    }
    count++;
    int x2 = x1 + (speed)*cos(anchor);
    int y2 = y1 + (speed)*sin(anchor);
    Pen penline(color, speed);
    penline.SetEndCap(LineCapRound);
    penline.SetStartCap(LineCapRound);
    mtx.lock();
    graphics->DrawLine(&penline, x1, y1, x2, y2);
    mtx.unlock();
    Sleep(1);
    if (clone > 0 && rand() % 10 == 0)
    {
        int x_copy = x1;
        int y_copy = y1;
        float anchor_copy = anchor + rand() % 10 / 26.0;
        float turn_copy = turn * (-1 + rand() % 2 * 2);
        int speed_copy = speed + rand() % 3;
        int cost_copy = cost + rand() % 3;
        thread t(vine, x_copy, y_copy, anchor_copy, turn_copy, speed_copy, cost_copy, Color(rand() % 50, 100 + rand() % 100, 20 + rand() % 50), 0, --clone);
        t.detach();
    }
    if (count % 10 == 0)
        speed -= cost;
    anchor += turn;
    if (count % 3 == 0)
        turn += turn / 2;
    if (turn > 0.6)
        speed = 0;
    vine(x2, y2, anchor, turn, speed, cost, color, count, clone);
    Sleep(count / 3);
    Pen erase(Color(255, 0, 0), speed * 2 + 3);
    erase.SetEndCap(LineCapRound);
    erase.SetStartCap(LineCapRound);
    mtx.lock();
    graphics->DrawLine(&erase, x1, y1, x2, y2);
    mtx.unlock();
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN) {
            // 获取鼠标点击位置
            POINT p;
            GetCursorPos(&p);
            float base = rand() % 10 / 3;
            thread t0(vine, p.x, p.y, 0 + base, 0.02, 5, 2, Color(0, 255, 0), 0, 3);
            t0.detach();
            thread t1(vine, p.x, p.y, 2 + base, 0.010, 5, 2, Color(0, 255, 0), 0, 3);
            t1.detach();
            thread t2(vine, p.x, p.y, -2 + base, 0.025, 5, 2, Color(0, 255, 0), 0, 3);
            t2.detach();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
