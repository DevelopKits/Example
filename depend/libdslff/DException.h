#pragma once

#ifdef WIN32
#include <windows.h>
#include <string>

#define	R_A_S	200 * 1024 * 1024	//保留地址空间。

class DException
{
public:
	DException();
	~DException();
	
	static int StartMonitor(const char* szFileName); 
	static int DoMiniDump(void* pExceptionInfo);
	
	
	
	//谨慎调用该函数，在不确定程序中还存在使用其他异常处理的情况下，不可调用。
	static int PreventOtherExceptionHandling(); 

	static bool m_firstException;	// 每个进程直触发一次dump
private:


	static int CreateMiniDumpFile();
	static void CloseMiniDumpFile();
	static int WriteMiniDumpFile(void* pExceptionInfo);
	static int DeleteExistDumpFile(const char* pcszDumpPath);

	static void ReleaseAddrsSpace();

	
private:
	static HANDLE m_hFileMiniDump;
	static std::string m_sFileName;
	static void* m_pAddrsSpace; //地址空间。

};

#else

class DException
{
public:
	DException(){};
	~DException(){};
	static int StartMonitor(const char* szFileName){return 0;}; 
	static int DoMiniDump(void* pExceptionInfo){return 0;};
	static int PreventOtherExceptionHandling(){return 0;}; 
};


#endif
