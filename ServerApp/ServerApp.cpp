// ServerApp.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "EventSocket.h"

int _tmain(int argc, _TCHAR* argv[])
{
	EventSocket* pEventSocket = new EventSocket;
	pEventSocket->StartServer(5001,10, 2 * 1000,2 * 60, 2 * 60);
	getchar();
	return 0;
}

