// SwartzClient.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "SwartzClient.h"


// ���ǵ���������һ��ʾ��
SWARTZCLIENT_API int nSwartzClient=0;

// ���ǵ���������һ��ʾ����
SWARTZCLIENT_API int fnSwartzClient(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� SwartzClient.h
CSwartzClient::CSwartzClient()
{
	return;
}
