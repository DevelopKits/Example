// SwartzServer.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "SwartzServer.h"


// ���ǵ���������һ��ʾ��
SWARTZSERVER_API int nSwartzServer=0;

// ���ǵ���������һ��ʾ����
SWARTZSERVER_API int fnSwartzServer(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� SwartzServer.h
CSwartzServer::CSwartzServer()
{
	return;
}
