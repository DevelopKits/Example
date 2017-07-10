// HookAllMsgBox.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "HookAllMsgBox.h"
#include <tchar.h>

HooKAllMsgBox g_HoolAllMsgBox;

void HookOn();
void HookOff();

//将变量放在共享段,及所有线程共享以下变量;
#pragma data_seg(".SHARED")
HINSTANCE g_hInstDll = NULL; //本dll的实例句柄;
HHOOK g_hMouse = NULL; //鼠标钩子句柄;
#pragma data_seg()
#pragma comment(linker, "/section:.SHARED,rws")

HWND g_hWnd = NULL;
HANDLE hProcess = NULL;
BOOL bIsInjected = FALSE;

//原函数定义
typedef int (WINAPI *MsgBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
typedef int (WINAPI *MsgBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
MsgBoxA oldMsgBoxA = NULL; //用于保存原函数地址
MsgBoxW oldMsgBoxW = NULL; //用于保存原函数地址
FARPROC pfMsgBoxA = NULL;//指向原函数地址的远指针
FARPROC pfMsgBoxW = NULL;//指向原函数地址的远指针
BYTE OldCodeA[5]; //老的系统API入口代码
BYTE NewCodeA[5]; //要跳转的API代码 (jmp xxxx)
BYTE OldCodeW[5]; //老的系统API入口代码
BYTE NewCodeW[5]; //要跳转的API代码 (jmp xxxx)

int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
int WINAPI MyMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstDll = hModule;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//鼠标钩子过程，目的是加载本dll到使用鼠标的程序
//鼠标钩子的作用：当鼠标在某程序窗口中时，其就会加载我们这个dll
LRESULT CALLBACK HookMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	//有鼠标消息时，将其发给主程序
	if (nCode == HC_ACTION)
	{
		::PostMessage(g_hWnd, WM_MYMOUSE, wParam, (LPARAM)(((PMOUSEHOOKSTRUCT)lParam)->hwnd));
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
int WINAPI UninitHook()
{
	HookOff();//记得恢复原API入口哈
	//主程序调用该函数时，恢复的只是主程序原API的入口，
	//其它程序的API入口还没有被恢复，所以我们必须处理
	//dll退出过程，即在函数ExitInstance()中，调用恢复
	//API入口的函数HookOff(),只有这样，其它程序再次调用
	//原API时，才不会发生错误喔。
	//当我们HOOK所有程序的某个系统API时，千万要注意在
	//ExitInstance()中调用HookOff(),血的教训哈。
	if (g_hMouse)
	{
		UnhookWindowsHookEx(g_hMouse);
		FreeLibrary(g_hInstDll);
		g_hMouse = NULL;
	}
	return 1;
}


 int WINAPI InitHook(HWND hWnd)
{
	g_hWnd = hWnd;
	g_hMouse = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)HookMouseProc, g_hInstDll, NULL);
	if (!g_hMouse)
	{
		UninitHook();
		return -1;
	}
	return 1;
}

// 开启钩子的函数
void HookOn()
{
	DWORD dwTemp = 0, dwOldProtect, dwRet = 0, dwWrite;

	//修改原API入口前五个字节为jmp xxxxxxxx
	//Debug版本在我这里修改失败，运行Release版本成功
	VirtualProtectEx(hProcess, pfMsgBoxA, 5, PAGE_READWRITE, &dwOldProtect);
	dwRet = WriteProcessMemory(hProcess, pfMsgBoxA, NewCodeA, 5, &dwWrite);
	if (0 == dwRet || 0 == dwWrite)
	{
		
	}
	VirtualProtectEx(hProcess, pfMsgBoxA, 5, dwOldProtect, &dwTemp);

	//修改原API入口前五个字节为jmp xxxxxxxx
	//Debug版本在我这里修改失败，运行Release版本成功
	VirtualProtectEx(hProcess, pfMsgBoxW, 5, PAGE_READWRITE, &dwOldProtect);
	dwRet = WriteProcessMemory(hProcess, pfMsgBoxW, NewCodeW, 5, &dwWrite);
	if (0 == dwRet || 0 == dwWrite)
	{
		
	}
	VirtualProtectEx(hProcess, pfMsgBoxW, 5, dwOldProtect, &dwTemp);
}

//关闭钩子的函数
void HookOff()//将所属进程中add()的入口代码恢复
{

	DWORD dwTemp = 0, dwOldProtect = 0, dwRet = 0, dwWrite = 0;

	//恢复原API入口
	VirtualProtectEx(hProcess, pfMsgBoxA, 5, PAGE_READWRITE, &dwOldProtect);
	dwRet = WriteProcessMemory(hProcess, pfMsgBoxA, OldCodeA, 5, &dwWrite);
	if (0 == dwRet || 0 == dwWrite)
	{
		//TRACE("啊，写入失败");
	}
	VirtualProtectEx(hProcess, pfMsgBoxA, 5, dwOldProtect, &dwTemp);

	//恢复原API入口
	VirtualProtectEx(hProcess, pfMsgBoxW, 5, PAGE_READWRITE, &dwOldProtect);
	WriteProcessMemory(hProcess, pfMsgBoxW, OldCodeW, 5, &dwWrite);
	if (0 == dwRet || 0 == dwWrite)
	{
		//TRACE("啊，写入失败");
	}
	VirtualProtectEx(hProcess, pfMsgBoxW, 5, dwOldProtect, &dwTemp);
}

HooKAllMsgBox::HooKAllMsgBox()
{
	DWORD dwPid = ::GetCurrentProcessId();
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);

	if (!bIsInjected)
	{
		bIsInjected = TRUE;//保证只调用1次
		//获取函数
		HMODULE hmod = ::LoadLibraryA(("User32.dll"));
		oldMsgBoxA = (MsgBoxA)::GetProcAddress(hmod, "MessageBoxA");
		pfMsgBoxA = (FARPROC)oldMsgBoxA;
		oldMsgBoxW = (MsgBoxW)::GetProcAddress(hmod, "MessageBoxW");
		pfMsgBoxW = (FARPROC)oldMsgBoxW;

		if (pfMsgBoxA == NULL)
		{
			MessageBoxA(NULL, ("cannot get MessageBoxA()"), ("error"), 0);
			return;
		}
		if (pfMsgBoxW == NULL)
		{
			MessageBoxA(NULL, ("cannot get MessageBoxW()"), ("error"), 0);
			return;
		}

		// 将原API中的入口代码保存入OldCodeA[]，OldCodeW[]
		_asm
		{
			lea edi, OldCodeA
				mov esi, pfMsgBoxA
				cld
				movsd
				movsb
		}
		_asm
		{
			lea edi, OldCodeW
				mov esi, pfMsgBoxW
				cld
				movsd
				movsb
		}

		NewCodeA[0] = 0xe9;//实际上0xe9就相当于jmp指令
		NewCodeW[0] = 0xe9;//实际上0xe9就相当于jmp指令

		//获取我们的API的地址
		_asm
		{
			lea eax, MyMessageBoxA
				mov ebx, pfMsgBoxA
				sub eax, ebx
				sub eax, 5
				mov dword ptr[NewCodeA + 1], eax
		}
		_asm
		{
			lea eax, MyMessageBoxW
				mov ebx, pfMsgBoxW
				sub eax, ebx
				sub eax, 5
				mov dword ptr[NewCodeW + 1], eax
		}

		//填充完毕，现在NewCode[]里的指令相当于Jmp Myadd
		HookOn(); //可以开启钩子了
	}
}

HooKAllMsgBox::~HooKAllMsgBox()
{
	//dll退出时，记得恢复原API入口哈，血的教训呀。
	//当我们钩所有程序的API，且dll退出没有恢复原API入口时，
	//那么当被钩程序再次调用该API时，会发生错误，因为我们的
	//dll程序已经退出了，原来API入口被修改的前五个字节还是
	//指向我们dll中的自己定义的函数地址，现在dll退出，该地址
	//自然也就不存在了，程序调用该自己时，自然会发生崩溃了。
	HookOff();
}

// 我们的假API函数MyMessageBoxA
int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	int nRet = 0;

	HookOff();//调用原函数之前，记得先恢复HOOK呀，不然是调用不到的
	//如果不恢复HOOK，就调用原函数，会造成死循环
	//毕竟调用的还是我们的函数，从而造成堆栈溢出，程序崩溃。

	nRet = ::MessageBoxA(hWnd, "哈哈，MessageBoxA被HOOK了吧", lpCaption, uType);
	nRet = ::MessageBoxA(hWnd, lpText, lpCaption, uType);
	HookOn();//调用完原函数后，记得继续开启HOOK，不然下次会HOOK不到哦。 

	return nRet;
}

//我们的假API函数MyMessageBoxW
int WINAPI MyMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	int nRet = 0;

	HookOff();//调用原函数之前，记得先恢复HOOK呀，不然是调用不到的
	//如果不恢复HOOK，就调用原函数，会造成死循环
	//毕竟调用的还是我们的函数，从而造成堆栈溢出，程序崩溃。

	nRet = ::MessageBoxW(hWnd, _T("哈哈,MessageBoxW被HOOK了吧"), lpCaption, uType);
	nRet = ::MessageBoxW(hWnd, lpText, lpCaption, uType);
	HookOn();//调用完原函数后，记得继续开启HOOK，不然下次会HOOK不到哦。 

	return nRet;
}
