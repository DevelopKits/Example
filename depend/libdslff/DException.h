#pragma once

#ifdef WIN32
#include <windows.h>
#include <string>

#define	R_A_S	200 * 1024 * 1024	//������ַ�ռ䡣

class DException
{
public:
	DException();
	~DException();
	
	static int StartMonitor(const char* szFileName); 
	static int DoMiniDump(void* pExceptionInfo);
	
	
	
	//�������øú������ڲ�ȷ�������л�����ʹ�������쳣���������£����ɵ��á�
	static int PreventOtherExceptionHandling(); 

	static bool m_firstException;	// ÿ������ֱ����һ��dump
private:


	static int CreateMiniDumpFile();
	static void CloseMiniDumpFile();
	static int WriteMiniDumpFile(void* pExceptionInfo);
	static int DeleteExistDumpFile(const char* pcszDumpPath);

	static void ReleaseAddrsSpace();

	
private:
	static HANDLE m_hFileMiniDump;
	static std::string m_sFileName;
	static void* m_pAddrsSpace; //��ַ�ռ䡣

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
