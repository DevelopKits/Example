// BaseLib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "BaseLib.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

BASELIB_EXPORTS IBaseLib* CreateBaseLib()
{
	return new BaseLibImpl;
}

BASELIB_EXPORTS void DestroyBaseLib(IBaseLib* pIBaseLib)
{
	pIBaseLib->Release();
}

int BaseLibImpl::Add(int a, int b)
{
	return a + b;
}

int BaseLibImpl::Sub(int a, int b)
{
	return a - b;
}

void BaseLibImpl::Release()
{
	delete this;
}
