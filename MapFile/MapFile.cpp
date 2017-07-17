// MapFile.cpp : 定义控制台应用程序的入口点。
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

	// 步骤1 打开文件FILE_FLAG_WRITE_THROUGH 
	HANDLE hFile = CreateFileA(
		"demo.txt",
		GENERIC_WRITE | GENERIC_READ,// 如果要映射文件：此处必设置为只读(GENERIC_READ)或读写
		0, // 此设为打开文件的任何尝试均将失败
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 
		NULL);
	if (hFile != INVALID_HANDLE_VALUE)// 文件打开失败返回句柄为-1
		// 这步必须测试，详细见步骤2
	{
		cout << "文件打开成功~!\n";
	}
	else
	{
		cout << "文件打开失败！\n";
		DWORD d = GetLastError();
		cout << d << endl;
		return -1;
	}

	// 步骤2 建立内存映射文件
	DWORD dwFileSize = GetFileSize(hFile, NULL);
	printf("文件大小为：%d\n", dwFileSize);
	HANDLE hFileMap = CreateFileMapping(
		hFile, // 如果这值为INVALID_HANDLE_VALUE,是合法的，上步一定测试啊
		NULL, // 默认安全性
		PAGE_READWRITE, // 可读写
		0, // 2个32位数示1个64位数，最大文件字节数，
		// 高字节，文件大小小于4G时，高字节永远为0
		0,//dwFileSize, // 此为低字节，也就是最主要的参数，如果为0，取文件真实大小
		NULL);
	if (hFileMap != NULL)
	{
		cout << "内存映射文件创建成功~!\n";
	}
	else
	{
		cout << "内存映射文件创建失败~！" << endl;
	}

	// 步骤3：将文件数据映射到进程的地址空间
	PVOID pvFile = MapViewOfFile( //pvFile就是得到的指针，用它来直接操作文件
		hFileMap,
		FILE_MAP_WRITE, // 可写
		0, // 文件指针头位置 高字节
		0, // 文件指针头位置 低字节 必为分配粒度的整倍数,windows的粒度为64K
		0); // 要映射的文件尾，如果为0，则从指针头到真实文件尾
	if (pvFile != NULL)
	{
		cout << "文件数据映射到进程的地址成功~!\n";
	}
	else
	{
		cout << "文件数据映射到进程的地址成功~!\n";
	}

	// 步骤4: 像操作内存一样操作文件,演示功能把整个文件倒序
	char *p = (char*)pvFile;
	printf("%s\n", p);
	for (unsigned int i = 0; i <= dwFileSize / 2; i++)
	{
		int nTmp = p[i];
		p[i] = p[dwFileSize - 1 - i];
		p[dwFileSize - 1 - i] = nTmp;
	}
	printf("%s\n", p);

	cout << "交换完毕，点击Enter退出" << endl;

	// 步骤5: 相关的释放工作
	UnmapViewOfFile(pvFile); // 释放内存映射文件的头指针
	CloseHandle(hFileMap); // 内存映射文件句柄
	CloseHandle(hFile); // 关闭文件
	getchar();
	return 0;
}

