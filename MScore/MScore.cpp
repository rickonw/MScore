//MScore.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MScore.h"
#include <tchar.h> 
#include <iostream>
#include <Winbase.h>
#include "toolkit.h"

using namespace vrv;

#define HMENU_BTN_INDEX 0

typedef enum
{
	HMENU_BTN_PLAY = HMENU_BTN_INDEX,
	HMENU_BTN_STOP,
	IDC_TIMER
};

#define MAX_LOADSTRING 100

#define MAX_DBG_MSG_LEN (1024)        
void DbgPrint(const char *format, ...)
{
	char buf[MAX_DBG_MSG_LEN], *p = buf;
	va_list args;
	va_start(args, format);
	p += _vsnprintf(p, sizeof(buf) - 1, format, args);
	va_end(args);
	OutputDebugStringA(buf);
}

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



#if true



#endif


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MSCORE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MSCORE));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	return (int)msg.wParam;
}

void btnPlayProc()
{


}

void btnStopProc()
{


}

void CALLBACK myTimerProc(
	HWND hwnd,
	UINT uMsg,
	UINT_PTR idEvent,
	DWORD dwTime)
{
	SYSTEMTIME sm;
	char tm[16];
	GetSystemTime(&sm);
	sprintf(tm, "%02d:%02d:%02d", sm.wHour + 8, sm.wMinute, sm.wSecond);//格式化输出结果
	DbgPrint("%s\r\n", tm);
}

//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MSCORE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MSCORE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   //LPWSTR 

   HWND btn_play = CreateWindow(
	   TEXT("BUTTON"),   // predefined class
	   TEXT("PLAY"),       // button text
	   WS_VISIBLE | WS_CHILD,  //values for buttons.
	   100,         // starting x position
	   100,         // starting y position
	   100,        // button width
	   20,        // button height
	   hWnd,       // parent window
	   (HMENU)HMENU_BTN_PLAY,       // No menu
	   (HINSTANCE)hInstance,
	   nullptr);
   HWND btn_stop = CreateWindow(
	   TEXT("BUTTON"),   // predefined class
	   TEXT("STOP"),       // button text
	   WS_VISIBLE | WS_CHILD,  //values for buttons.
	   100,         // starting x position
	   200,         // starting y position
	   100,        // button width
	   20,        // button height
	   hWnd,       // parent window
	   (HMENU)HMENU_BTN_STOP,       // No menu
	   (HINSTANCE)nullptr,
	   nullptr);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		SetTimer(hWnd, IDC_TIMER, 1000, myTimerProc);//设置定时器
		break;
	case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case HMENU_BTN_PLAY:
				btnPlayProc();
				break;
			case HMENU_BTN_STOP:
				btnStopProc();
				break;
			default:
				DbgPrint("wmId = %d\r\n", wmId);
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
		KillTimer(hWnd, IDC_TIMER);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

////


