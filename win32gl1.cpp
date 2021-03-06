// win32gl1.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "win32gl1.h"
#include "printText.h"
#include "loadpng.h"
#include "gamelogic.h"
#include "director.h"
#include "testscene1.h"
#include "menuscene.h"
#include "simplegame1.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
using namespace std;

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
HDC hDC;
PIXELFORMATDESCRIPTOR  pfd;
HGLRC ghRC;
bool pauseFlag;
bool openglInited = false;
LPDWORD DrawingThread = NULL;
HANDLE DrawingThreadHANDLE = NULL;
HWND hWnd;
float FPS = 60;
float LastCycleTime = 0.01;//上一帧的时间
POINT mousepos;//鼠标位置
char lastPressedKey = '\0';
bool shouldRun = false;



// 此代码模块中包含的函数的前向声明: 
void  SceneShow(GLvoid);

void  SceneInit(int w, int h);

void openglInit();//初始化OPENGL

void showFPS();//显示FPS

void setFPS(float fps);//设置FPS

void getLastCycleTime();//获得上一帧的时间（注意！每个循环只能调用一次！！！）

void getKeyBoardState();



DWORD WINAPI  DrawingLoop(LPVOID lpParameter);

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO:  在此放置代码。

	pauseFlag = false;

	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32GL1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32GL1));

	// 主消息循环: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32GL1));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_WIN32GL1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	Director::create();
	hInst = hInstance; // 将实例句柄存储在全局变量中
	//设置窗口大小
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW&~WS_THICKFRAME,
		200, 200, WINDOW_WIDTH + 18, WINDOW_HEIGHT+61, NULL, NULL, hInstance, NULL);
	//~WS_THICKFRAME禁止改变窗口大小
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;


	switch (message)
	{
	case WM_COMMAND:
		SceneShow();
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_PAUSE:
			pauseFlag = true;
			break;
		case IDM_RESUME:
			pauseFlag = false;
			break;
		case IDM_START:
			shouldRun = true;
			if (!DrawingThreadHANDLE)
				DrawingThreadHANDLE = CreateThread(NULL, 0, DrawingLoop, NULL, 0, DrawingThread);
			break;
		case IDM_END:
			shouldRun = false;
			TerminateThread(DrawingThreadHANDLE, 0);
			DrawingThreadHANDLE = NULL;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		// TODO:  在此添加任意绘图代码...

		break;
	case WM_DESTROY:
		shouldRun = false;
		TerminateThread(DrawingThreadHANDLE, 0);
		PostQuitMessage(0);
		exit(0);
		break;
	case WM_LBUTTONDOWN:

		break;
	case WM_CHAR:
		lastPressedKey = wParam;
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


//OpenGL 初始化开始 

void  SceneInit(int  w, int  h)

{
	BuildFont(hDC);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);        //   黑色背景 

	glColor3f(1.0f, 1.0f, 1.0f);



	//glShadeModel(GL_FLAT);

	//glMatrixMode(GL_MODELVIEW);

	//glLoadIdentity();

	//glOrtho(-50.0f, 50.0f, -50.0f, 50.0f, -10.0f, 10.0f);

}



//  这里进行所有的绘图工作 

void  SceneShow(GLvoid)

{

	GetCursorPos(&mousepos);//获得鼠标位置

	getLastCycleTime();//每帧重置一下计时器

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();


	update(0.1f);
	if (Director::getTheInstance()->getTotalScene())
	{
		Director::getTheInstance()->getKeyState();
		Director::getTheInstance()->tickTimers();
		Scene* thescene = Director::getTheInstance()->getCurrentScene();
		thescene->update(1 / 60.0);
		sort(thescene->getSpriteList().begin(), thescene->getSpriteList().end());
		for (vector<Sprite*>::iterator it = thescene->getSpriteList().begin(); it != thescene->getSpriteList().end(); it++)
		{
			Point origin = (*it)->getpos();
			Point anchorPoint = (*it)->getAnchorPoint();
			Size size = (*it)->getsize();
			swap(size.height, size.width);
			float rot = (*it)->getrotation();
			glLoadIdentity();


			float r = sqrt(size.width*size.width + size.height*size.height);

			Point bl, br, tr, tl;

			bl.x = -(size.width) / r;
			bl.y = -(size.height) / r;

			br.x = (size.width) / r;
			br.y = -(size.height) / r;

			tr.x = (size.width) / r;
			tr.y = (size.height) / r;

			tl.x = -(size.width) / r;
			tl.y = (size.height) / r;



			glRotatef(90.0f + rot/M_PI*180, 0.0f, 0.0f, 1.0f);
			//TODO:FIX IT
			glViewport(origin.x - r *anchorPoint.x, origin.y - r*anchorPoint.y, r, r);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glMatrixMode(GL_MODELVIEW);
			if ((*it)->getflipX())                                  //左右翻转
			{
				swap(bl.x, br.x);
				swap(bl.y, br.y);
				swap(tl.x, tr.x);
				swap(tl.y, tr.y);
			}

			if ((*it)->getflipY())                                  //上下翻转
			{
				swap(bl.x, tl.x);
				swap(bl.y, tl.y);
				swap(br.x, tr.x);
				swap(br.y, tr.y);;
			}
			glBindTexture(GL_TEXTURE_2D, (*it)->getTex());          //与纹理绑定
			glBegin(GL_POLYGON);

			glTexCoord2f(0.0f, 0.0f); glVertex3f(bl.x, bl.y, 0.0f);    //左下
			glTexCoord2f(0.0f, 1.0f); glVertex3f(br.x, br.y, 0.0f);      //右下
			glTexCoord2f(1.0f, 1.0f); glVertex3f(tr.x, tr.y, 0.0f);      //右上
			glTexCoord2f(1.0f, 0.0f); glVertex3f(tl.x, tl.y, 0.0f);      //左上

			glEnd();
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}
	}
	///////////////////////////////////////////////////////////
	//任何glPrint应在此行之后执行（不然看不到）
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glLoadIdentity();
	glRasterPos2f(-1.0f, -0.8f);
	ScreenToClient(hWnd, &mousepos);
	Director::getTheInstance()->setMousePos(mousepos);
	glPrint("mouse pos(%ld,%ld)", mousepos.x, mousepos.y);
	glRasterPos2f(-1.0f, -0.6f);
	glPrint("Keyboard: %c", lastPressedKey);
	//   交换缓冲区 
	getKeyBoardState();
	showFPS();
	glRasterPos2f(-1.0f, -0.5f);
	glPrint("aswd control r reset");
	SwapBuffers(hDC);

}

void initDrawing()
{
	int  iFormat;



	hDC = GetDC(hWnd);



	ZeroMemory(&pfd, sizeof (pfd));

	pfd.nSize = sizeof (pfd);

	pfd.nVersion = 1;        //  版本，一般设为 



	//  一组表明象素缓冲特性的标志位 

	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	pfd.iPixelType = PFD_TYPE_RGBA;     //  明象素数据类型是RGBA 还是颜色索引;  

	pfd.cColorBits = 32;       //  每个颜色缓冲区中颜色位平面的数目，对颜色索引方式是缓冲区大小 

	pfd.cDepthBits = 16;

	pfd.iLayerType = PFD_MAIN_PLANE;  //  被忽略，为了一致性而包含的 



	iFormat = ChoosePixelFormat(hDC, &pfd); //  选择一个像素格式 



	SetPixelFormat(hDC, iFormat, &pfd);  //  设置到DC 中 



	ghRC = wglCreateContext(hDC);      //  创建绘图描述表 

	wglMakeCurrent(hDC, ghRC);       //  使之成为当前绘图描述表 


	openglInit();
}

DWORD WINAPI  DrawingLoop(LPVOID lpParameter)
{
	if (!openglInited)
		initDrawing();
	while (shouldRun)
	{
		SceneShow();
		Sleep(1000 / 60);
	}
	return 0;
}

//OpenGL 初始化开始 

void openglInit()
{
	BuildFont(hDC);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);        //   黑色背景 

	glColor3f(1.0f, 1.0f, 1.0f);
	glShadeModel(GL_FLAT);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	///////////////////////////////////////////
	//在此加载场景
	Scene* p = (Scene*)new testscene1("test");
	Scene* p2 = (Scene*)new menuScene("menu");
	Scene* p3 = (Scene*)new simplegame1("game1");
	Director::getTheInstance()->addScene(p2);
	Director::getTheInstance()->addScene(p);
	Director::getTheInstance()->addScene(p3);
	Director::getTheInstance()->startWithScene("game1");
}

void showFPS()
{
	glLoadIdentity();
	glRasterPos2f(-1.0f, -1.0f);

	static float lastFPS = 0;
	lastFPS = 1.0 / LastCycleTime;
	glPrint("FPS:%f", lastFPS);
}


void setFPS(float fps)
{
	FPS = fps;
}

void getLastCycleTime()
{
	LARGE_INTEGER LARGE;
	static LONGLONG lasttimeL = 0;
	LONGLONG nowL = 0;
	double freq = 0;
	QueryPerformanceFrequency(&LARGE);
	freq = (double)LARGE.QuadPart;//获得频率
	QueryPerformanceCounter(&LARGE);
	nowL = LARGE.QuadPart;
	LastCycleTime = ((float)(nowL - lasttimeL)) / freq;
	lasttimeL = nowL;
}

void getKeyBoardState()
{
	if (true)
	{
		glRasterPos2f(-1.0f, -0.2f);
		glPrint("L button Pressed %d", GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	}
}