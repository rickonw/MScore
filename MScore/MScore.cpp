//MScore.cpp: 定义应用程序的入口点。
//

//#include "afxwin.h"
#include<windows.h>
#include "stdafx.h"
#include "MScore.h"
#include <tchar.h> 
#include <iostream>
#include <Winbase.h>
#include "toolkit.h"
#include "resource.h"
#include <comdef.h>


//svgren
#include <svgren/render.hpp>
#include <papki/FSFile.hpp> //we will need this to load the SVG file

//#pragma comment( lib, "afxwin.lib" )
//GDI
//#include "GdiPlus.h" 
//using namespace Gdiplus;
#pragma comment( lib, "gdiplus.lib" )

//#include <afxwin.h>

//打开保存文件对话框
#include<commdlg.h>

//选择文件夹对话框
#include<shlobj.h>
//#pragma comment(lib,"Shell32.lib")
#include <process.h>     //for _beginthreadex

using namespace vrv;
using namespace std;


/* 外部函数 */
extern void WMPlay_Pause(bool flag);

#define HMENU_BTN_INDEX 0

unsigned long g_ticks = 0;

typedef enum
{
	IDC_TIMER = HMENU_BTN_INDEX
};

#define MAX_LOADSTRING 100

//n(ms)
#define TICK_PERIOD 10

HANDLE threadCtrl = NULL;
DWORD  threadCtrlID;
HANDLE hStartEvent;
HANDLE threadPlay = NULL;
DWORD  threadPlayID;

vrv::Toolkit *vrvToolkit = NULL;
std::string playingMidifile;

bool playTimer = false;
bool wmPlaying = false;
LPARAM playStartTime = 0;

//CStatic m_PictureBox;

typedef enum
{
	THREAD_MSG_TIME = WM_USER + 100,
	THREAD_MSG_OPENFILE,
	THREAD_MSG_STOP,
	THREAD_MSG_PLAY,
	THREAD_MSG_PLAY_MUTE,
};

const int MAX_INFO_SIZE = MAX_PATH;

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

char* TCHAR2CHAR(LPWSTR lpwszStrIn)
{
	LPSTR pszOut = NULL;
	if (lpwszStrIn != NULL)
	{
		int nInputStrLen = wcslen(lpwszStrIn);

		// Double NULL Termination  
		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
		pszOut = new char[nOutputStrLen];

		if (pszOut)
		{
			memset(pszOut, 0x00, nOutputStrLen);
			WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
		}
	}
	return pszOut;
}

std::string TCHAR2STRING(TCHAR *STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0,STR, -1, NULL, 0, NULL, NULL);
	char* chRtn =new char[iLen*sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
	std::string str(chRtn);
	return str;
}


// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND hdlgMain;

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

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

//svgren
void svgrenLoad( )
{
	auto dom = svgdom::load(papki::FSFile("camera.svg"));

	unsigned width = 0; //0 means use width from SVG document
	unsigned height = 0; //0 means use height from SVG document
	auto img = svgren::render(*dom, width, height); //uses 96 dpi by default
	

}

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300

void Paint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HDC mdc;
	HBITMAP hbmp; // 位图绘制对象句柄
	const WCHAR *path = L"D:\\work\\GitHub\\projects\\MScore\\x64\\Debug\\Afrique.bmp";
	const char *svg_path = "D:\\work\\GitHub\\projects\\MScore\\x64\\Debug\\importer.svg";

	hdc = BeginPaint(hwnd, &ps);

	// 创建缓存DC (当前窗口DC的兼容DC)
	mdc = CreateCompatibleDC(hdc);

	auto dom = svgdom::load(papki::FSFile(svg_path));

	unsigned width = 0; //0 means use width from SVG document
	unsigned height = 0; //0 means use height from SVG document
	auto img = svgren::render(*dom, width, height); //uses 96 dpi by default

	// 加载 "E:\bg.bmp" 到位图绘制对象 hbmp 中
	hbmp = (HBITMAP)LoadImage(
		NULL,           // 模块实例句柄(要加载的图片在其他DLL中时)
		path,    // 位图路径
		IMAGE_BITMAP,   // 位图类型
		WINDOW_WIDTH,   // 指定图片宽
		WINDOW_HEIGHT,  // 指定图片高
		LR_LOADFROMFILE // 从路径处加载图片
	);
	//LPCWSTR
	// 缓存DC选择位图绘制对象（可以理解为将图片存到mdc中）
	SelectObject(mdc, hbmp);

	// 将缓存DC中的位图复制到窗口DC上
	BitBlt(
		hdc,            // 目的DC
		0, 0,            // 目的DC的 x,y 坐标
		WINDOW_WIDTH,   // 要粘贴的图片宽
		WINDOW_HEIGHT,  // 要粘贴的图片高
		mdc,            // 缓存DC
		0, 0,            // 缓存DC的 x,y 坐标
		SRCCOPY         // 粘贴方式
	);

	//CBitmap Chb;
	//HBITMAP bmp = CreateBitmap(width, height, 1, 32, &*img.data());
	//ASSERT_ALWAYS(bmp != NULL)
	//Chb.Attach(bmp);
	//PicControl.ModifyStyle(0xF, SS_BITMAP, SWP_NOSIZE);
	//PicControl.SetBitmap(Chb);
	//mProjectorWindow.m_picControl.ModifyStyle(0xF, SS_BITMAP, SWP_NOSIZE);
	//mProjectorWindow.m_picControl.SetBitmap(Chb);
	
	

	DeleteObject(hbmp);
	DeleteDC(mdc);
	EndPaint(hwnd, &ps);
}

//TODO 打开midi文件
extern vrv::Toolkit * MScore_OpenFile(std::string infile, std::string &outfile);
extern int MScore_Play(vrv::Toolkit &toolkit, std::string outfile);

DWORD WINAPI threadPlayProc(LPVOID lpParameter) {
	char * pInfo;
	TCHAR* tcFile;

	DbgPrint("threadPlayProc\r\n");

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	if (!SetEvent(hStartEvent)) //set thread start event 
	{
		DbgPrint("set start event failed,errno:%d\n", ::GetLastError());
		return 1;
	}

	while (true)
	{
		if (GetMessage(&msg, 0, 0, 0)) //get msg from message queue
		{
			switch (msg.message)
			{
			case THREAD_MSG_PLAY:
                MScore_Play(*vrvToolkit, playingMidifile);
                wmPlaying = true;
				break;
			case THREAD_MSG_PLAY_MUTE:
                playTimer = true;
				playStartTime = g_ticks;
			    break;
			
			}
		}
	};

	return 0;
}

DWORD WINAPI threadCtrlProc(LPVOID lpParameter) {
	char * pInfo;
	TCHAR* tcFile;
	string elem;
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	if (!SetEvent(hStartEvent)) //set thread start event 
	{
		DbgPrint("set start event failed,errno:%d\n", ::GetLastError());
		return 1;
	}

	while (true)
	{
		if (GetMessage(&msg, 0, 0, 0)) //get msg from message queue
		{
			switch (msg.message)
			{
			case THREAD_MSG_TIME:
				//DbgPrint("recv tick %d\r\n", msg.lParam);
                if (!playTimer) {
                    break;
                }

				elem = vrvToolkit->GetElementsAtTime(msg.lParam - playStartTime);
				
				break;
			case THREAD_MSG_OPENFILE:
				tcFile = (TCHAR *)msg.wParam;
				OutputDebugString(tcFile);
				string strFile = TCHAR2STRING(tcFile);
				if(vrvToolkit) {
                    delete vrvToolkit;
                    vrvToolkit = NULL;
				}

				vrvToolkit = MScore_OpenFile(strFile, playingMidifile);
				if (!vrvToolkit) {
					MessageBox(NULL, TEXT("乐谱文件打开失败"), NULL, MB_ICONERROR);
				}
				
				break;
			}
		}
	};

	return 0;
}

void btnPlayProc()
{
	HWND hwnd;

    if(!vrvToolkit) {
        MessageBox(NULL, TEXT("请打开一个文件"), NULL, MB_ICONERROR);
        return;
    }

    if (!threadPlay) {
        hStartEvent = ::CreateEvent(0, FALSE, FALSE, 0); //create thread start event
        if (NULL == hStartEvent) {
            printf("create start event failed, errno:%d\n", ::GetLastError());
            return;
        }
    
        threadPlay = CreateThread(NULL, 0, threadPlayProc, NULL, 0, &threadPlayID);
        if (NULL == threadPlay) {
            CloseHandle(hStartEvent);
            printf("create thread failed, errno:%d\n", ::GetLastError());
            return;
        }
    
        ::WaitForSingleObject(hStartEvent, INFINITE);
        CloseHandle(hStartEvent);
    }

	//hwnd = GetDlgItem(hdlgMain, IDC_CHECK_ACCOMPANY);
	int state = 1;
	WMPlay_Pause(false);
	//CheckDlgButton
	
	//state = ((CButton *)GetDlgItem(hdlgMain, IDC_CHECK_ACCOMPANY))->GetCheck();
	if(::IsDlgButtonChecked(hdlgMain, IDC_CHECK_ACCOMPANY)) {
		PostThreadMessage(threadPlayID, THREAD_MSG_PLAY, (WPARAM)0, 0);
	}
	else {
		PostThreadMessage(threadPlayID, THREAD_MSG_PLAY_MUTE, (WPARAM)0, 0);
	}
}

void btnStopProc()
{
    if(playTimer) {
        playTimer = false;
    }
    if (wmPlaying) {
        WMPlay_Pause(true);
    }
    wmPlaying = false;
    
}

void menuOpenFile()
{
    OPENFILENAME ofn = { 0 };
    TCHAR strFilename[MAX_PATH] = { 0 };//用于接收文件名
    ofn.lStructSize = sizeof(OPENFILENAME);//结构体大小
    ofn.hwndOwner = NULL;//拥有着窗口句柄，为NULL表示对话框是非模态的，实际应用中一般都要有这个句柄
    ofn.lpstrFilter = TEXT("所有文件\0*.*\0C/C++ Flie\0*.cpp;*.c;*.h\0\0");//设置过滤
    ofn.nFilterIndex = 1;//过滤器索引
    ofn.lpstrFile = strFilename;//接收返回的文件名，注意第一个字符需要为NULL
    ofn.nMaxFile = sizeof(strFilename);//缓冲区长度
    ofn.lpstrInitialDir = NULL;//初始目录为默认
    ofn.lpstrTitle = TEXT("请选择一个文件");//使用系统默认标题留空即可
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;//文件、目录必须存在，隐藏只读选项
    if (GetOpenFileName(&ofn))
    {
        //MessageBox(NULL, strFilename, TEXT("选择的文件"), 0);
    }
    else{
        return;
    }

	TCHAR *pInfo = new TCHAR[MAX_INFO_SIZE];
	memcpy(pInfo, strFilename, MAX_INFO_SIZE);
	if (!PostThreadMessage(threadCtrlID, THREAD_MSG_OPENFILE, (WPARAM)pInfo, 0)) //post thread msg
	{
		DbgPrint("post message failed, errno:%d\n", ::GetLastError());
		delete []pInfo;
	}

    return;
}

void menuSaveFile()
{
    loop:
    OPENFILENAME ofn = { 0 };
    TCHAR strFilename[MAX_PATH] = { 0 };//用于接收文件名
    ofn.lStructSize = sizeof(OPENFILENAME);//结构体大小
    ofn.hwndOwner = NULL;//拥有着窗口句柄，为NULL表示对话框是非模态的，实际应用中一般都要有这个句柄
    ofn.lpstrFilter = TEXT("所有文件\0*.*\0C/C++ Flie\0*.cpp;*.c;*.h\0\0");//设置过滤
    ofn.nFilterIndex = 1;//过滤器索引
    ofn.lpstrFile = strFilename;//接收返回的文件名，注意第一个字符需要为NULL
    ofn.nMaxFile = sizeof(strFilename);//缓冲区长度
    ofn.lpstrInitialDir = NULL;//初始目录为默认
    ofn.lpstrTitle = TEXT("请选择一个文件");//使用系统默认标题留空即可
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;//文件、目录必须存在，隐藏只读选项
    if (GetOpenFileName(&ofn))
    {
        MessageBox(NULL, strFilename, TEXT("选择的文件"), 0);
    }
    else{
        MessageBox(NULL, TEXT("请选择一个文件"), NULL, MB_ICONERROR);
        goto loop;
    }

    ofn.Flags =  OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;//目录必须存在，覆盖文件前发出警告
    ofn.lpstrTitle = TEXT("保存到");//使用系统默认标题留空即可
    ofn.lpstrDefExt = TEXT("cpp");//默认追加的扩展名
    if (GetSaveFileName(&ofn))
    {
        MessageBox(NULL, strFilename, TEXT("保存到"), 0);
    }
    else{
        MessageBox(NULL, TEXT("请输入一个文件名"), NULL, MB_ICONERROR);
    }

    TCHAR szBuffer[MAX_PATH] = { 0 };
    BROWSEINFO bi = { 0 };
    bi.hwndOwner = NULL;//拥有着窗口句柄，为NULL表示对话框是非模态的，实际应用中一般都要有这个句柄
    bi.pszDisplayName = szBuffer;//接收文件夹的缓冲区
    bi.lpszTitle = TEXT("选择一个文件夹");//标题
    bi.ulFlags = BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST idl = SHBrowseForFolder(&bi);
    if (SHGetPathFromIDList(idl, szBuffer)){
        MessageBox(NULL, szBuffer, TEXT("你选择的文件夹"), 0);
    }
    else{
        MessageBox(NULL, TEXT("请选择一个文件夹"), NULL, MB_ICONERROR);
    }

    return;
}

void CALLBACK myTimerProc(
	HWND hwnd,
	UINT uMsg,
	UINT_PTR idEvent,
	DWORD dwTime)
{
	//SYSTEMTIME sm;
	//char tm[16];
	//GetSystemTime(&sm);
	//sprintf(tm, "%02d:%02d:%02d", sm.wHour + 8, sm.wMinute, sm.wSecond);//格式化输出结果
	//DbgPrint("%s\r\n", tm);
	g_ticks++;
	
	if (!PostThreadMessage(threadCtrlID, THREAD_MSG_TIME, 0, g_ticks))
	{
		printf("post message failed, errno:%d\n", ::GetLastError());
	}
}

INT_PTR CALLBACK CtrlMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:

		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDC_BUTTON_PLAY:
			btnPlayProc();
			break;
		case IDC_BUTTON_STOP:
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
		//HWND picHwnd = GetDlgItem(hWnd, IDC_STATIC_PIC);
		
		//HDC hdc = BeginPaint(picHwnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		Paint(hWnd);
		
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
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
      CW_USEDEFAULT, CW_USEDEFAULT, 800, 400, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

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
		// 创建对话框
		hdlgMain = CreateDialog(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, CtrlMsgProc);
		SetWindowPos(hdlgMain, hWnd, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, SWP_NOZORDER | SWP_NOMOVE);
		ShowWindow(hdlgMain, SW_SHOWNA);

		if (!threadCtrl) {
			hStartEvent = ::CreateEvent(0, FALSE, FALSE, 0); //create thread start event
			if (NULL == hStartEvent) {
				printf("create start event failed, errno:%d\n", ::GetLastError());
				return 1;
			}

			threadCtrl = CreateThread(NULL, 0, threadCtrlProc, NULL, 0, &threadCtrlID);
			if (NULL == threadCtrl) {
				CloseHandle(hStartEvent);
				printf("create thread failed, errno:%d\n", ::GetLastError());
				return 1;
			}

			::WaitForSingleObject(hStartEvent, INFINITE);
			CloseHandle(hStartEvent);
		}

		SetTimer(hWnd, IDC_TIMER, TICK_PERIOD, myTimerProc);//设置定时器
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
			case IDM_OPEN:
				menuOpenFile();
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
		CloseHandle(threadCtrl);
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





