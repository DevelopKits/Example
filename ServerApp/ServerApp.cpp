// ServerApp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "EventSocket.h"

int _tmain(int argc, _TCHAR* argv[])
{
	EventSocket* pEventSocket = new EventSocket;
	pEventSocket->StartServer(5000,20000,5, 5);
	getchar();
	return 0;
}

