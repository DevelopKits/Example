// BaseTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "BaseLib.h"

#pragma comment(lib, "Bin/BaseLib.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	IBaseLib* pBaseLib = CreateBaseLib();
	int nret = pBaseLib->Add(10, 20);
	nret = pBaseLib->Sub(100, 20);
	DestroyBaseLib(pBaseLib);
	return 0;
}

