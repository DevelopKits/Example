// MapFile.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
	char strExePathTmp[260] = { 0 };
	GetModuleFileNameA(NULL, strExePathTmp, 260);
	std::string strExePath(strExePathTmp);
	strExePath.resize(strExePath.find_last_of('\\'));
	SetCurrentDirectoryA(strExePath.c_str());

	// ����1 ���ļ�FILE_FLAG_WRITE_THROUGH 
	HANDLE hFile = CreateFileA(
		"demo.txt",
		GENERIC_WRITE | GENERIC_READ,// ���Ҫӳ���ļ����˴�������Ϊֻ��(GENERIC_READ)���д
		0, // ����Ϊ���ļ����κγ��Ծ���ʧ��
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 
		NULL);
	if (hFile != INVALID_HANDLE_VALUE)// �ļ���ʧ�ܷ��ؾ��Ϊ-1
		// �ⲽ������ԣ���ϸ������2
	{
		cout << "�ļ��򿪳ɹ�~!\n";
	}
	else
	{
		cout << "�ļ���ʧ�ܣ�\n";
		DWORD d = GetLastError();
		cout << d << endl;
		return -1;
	}

	// ����2 �����ڴ�ӳ���ļ�
	DWORD dwFileSize = GetFileSize(hFile, NULL);
	printf("�ļ���СΪ��%d\n", dwFileSize);
	HANDLE hFileMap = CreateFileMapping(
		hFile, // �����ֵΪINVALID_HANDLE_VALUE,�ǺϷ��ģ��ϲ�һ�����԰�
		NULL, // Ĭ�ϰ�ȫ��
		PAGE_READWRITE, // �ɶ�д
		0, // 2��32λ��ʾ1��64λ��������ļ��ֽ�����
		// ���ֽڣ��ļ���СС��4Gʱ�����ֽ���ԶΪ0
		0,//dwFileSize, // ��Ϊ���ֽڣ�Ҳ��������Ҫ�Ĳ��������Ϊ0��ȡ�ļ���ʵ��С
		NULL);
	if (hFileMap != NULL)
	{
		cout << "�ڴ�ӳ���ļ������ɹ�~!\n";
	}
	else
	{
		cout << "�ڴ�ӳ���ļ�����ʧ��~��" << endl;
	}

	// ����3�����ļ�����ӳ�䵽���̵ĵ�ַ�ռ�
	PVOID pvFile = MapViewOfFile( //pvFile���ǵõ���ָ�룬������ֱ�Ӳ����ļ�
		hFileMap,
		FILE_MAP_WRITE, // ��д
		0, // �ļ�ָ��ͷλ�� ���ֽ�
		0, // �ļ�ָ��ͷλ�� ���ֽ� ��Ϊ�������ȵ�������,windows������Ϊ64K
		0); // Ҫӳ����ļ�β�����Ϊ0�����ָ��ͷ����ʵ�ļ�β
	if (pvFile != NULL)
	{
		cout << "�ļ�����ӳ�䵽���̵ĵ�ַ�ɹ�~!\n";
	}
	else
	{
		cout << "�ļ�����ӳ�䵽���̵ĵ�ַ�ɹ�~!\n";
	}

	// ����4: ������ڴ�һ�������ļ�,��ʾ���ܰ������ļ�����
	char *p = (char*)pvFile;
	printf("%s\n", p);
	for (unsigned int i = 0; i <= dwFileSize / 2; i++)
	{
		int nTmp = p[i];
		p[i] = p[dwFileSize - 1 - i];
		p[dwFileSize - 1 - i] = nTmp;
	}
	printf("%s\n", p);

	cout << "������ϣ����Enter�˳�" << endl;

	// ����5: ��ص��ͷŹ���
	UnmapViewOfFile(pvFile); // �ͷ��ڴ�ӳ���ļ���ͷָ��
	CloseHandle(hFileMap); // �ڴ�ӳ���ļ����
	CloseHandle(hFile); // �ر��ļ�
	getchar();
	return 0;
}

