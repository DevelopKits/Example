// ServerApp.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "EventSocket.h"

int _tmain(int argc, _TCHAR* argv[])
{
	EventSocket* pEventSocket = new EventSocket;
	pEventSocket->StartServer(5000,1, 100,2 * 60, 2 * 60);
	getchar();
	return 0;
}

