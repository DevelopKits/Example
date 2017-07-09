// KeyMouseHook.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <stdio.h>
#include "KeyMouseHook.h"


//将变量放在共享段,及所有线程共享以下变量;
#pragma data_seg(".SHARED")
HINSTANCE g_hInstDll = NULL; //本dll的实例句柄;
HHOOK g_hKeybd = NULL; //键盘钩子句柄;
HHOOK g_hMouse = NULL; //鼠标钩子句柄;
#pragma data_seg()
#pragma comment(linker, "/section:.SHARED,rws")

HWND g_hWnd = NULL;

BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		g_hInstDll = hModule;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

LRESULT CALLBACK HookMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	//有鼠标消息时，将其发给主程序
	if (nCode == HC_ACTION)
	{
		::PostMessage(g_hWnd, WM_MYMOUSE, wParam, lParam);
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK HookKeybdProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// 有键盘消息时，将其发给主程序
	if (nCode == HC_ACTION)
	{
		::PostMessage(g_hWnd, WM_MYKEY, wParam, lParam);
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

KEYMOUSEHOOK_API int InitHook(HWND hWnd)
{
	g_hWnd = hWnd;
	g_hKeybd = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)HookKeybdProc, g_hInstDll, NULL);
	if (!g_hKeybd)
	{
		UninitHook();
		return -1;
	}

	g_hMouse = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)HookMouseProc, g_hInstDll, NULL);
	if (!g_hMouse)
	{
		UninitHook();
		return -1;
	}

	return 1;
}

KEYMOUSEHOOK_API int UninitHook()
{
	if (g_hKeybd)
	{
		UnhookWindowsHookEx(g_hKeybd);
		g_hKeybd = NULL;
	}

	if (g_hMouse)
	{
		UnhookWindowsHookEx(g_hMouse);
		g_hMouse = NULL;
	}

	return 1;
}

