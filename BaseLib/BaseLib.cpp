// BaseLib.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "BaseLib.h"

//���еĵ��ø�dll�ĵط��Ṳ��ñ����������Ǹ���һ������
#pragma data_seg(".Segment")
int test = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.Segment,rws")


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
	test = test+ a + b;
	return test;
}

int BaseLibImpl::Sub(int a, int b)
{
	return a - b;
}

void BaseLibImpl::Release()
{
	delete this;
}
