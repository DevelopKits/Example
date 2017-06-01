// BaseTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "BaseLib.h"
#include <windows.h>
#pragma comment(lib, "Bin/BaseLib.lib")

void OutputDebugPrintf(const char * strOutputString, ...)
{
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugString(strBuffer);
}

int _tmain(int argc, _TCHAR* argv[])
{
	IBaseLib* pBaseLib = CreateBaseLib();
	int nret = pBaseLib->Add(10, 20);
	OutputDebugPrintf("DEBUGINFO| result = %d", nret);
	nret = pBaseLib->Sub(100, 20);
	OutputDebugPrintf("DEBUGINFO| result = %d", nret);
	DestroyBaseLib(pBaseLib);
	while (true)
	{
		Sleep(100);
	}
	return 0;
}

