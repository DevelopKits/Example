#include "GetBaseInfo.h"


GetBaseInfo::GetBaseInfo(QDialog* parent )
{
	ui.setupUi(this);
	connect(ui.m_btnStart, SIGNAL(clicked()), this, SLOT(onStart()));
	connect(ui.m_btnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
}


GetBaseInfo::~GetBaseInfo()
{
}

void  GetAllMemoryInfo(DWORDLONG& dwMemoryLoad, DWORDLONG& dwAviPhyMBSize, DWORDLONG& dwAviVirMBSize)
{
	//   TODO:     Add   extra   initialization   here   
	MEMORYSTATUSEX   Mem;
	ZeroMemory(&Mem, sizeof(MEMORYSTATUSEX));
	Mem.dwLength = sizeof(MEMORYSTATUSEX);
	//   get   the   memory   status    
	GlobalMemoryStatusEx(&Mem);
	dwMemoryLoad = Mem.dwMemoryLoad;

	DWORDLONG dwSize = (DWORDLONG)Mem.ullTotalPhys;
	DWORDLONG dwVirtSize = (DWORDLONG)Mem.ullTotalVirtual;
	dwAviPhyMBSize = Mem.ullAvailPhys;
	dwAviVirMBSize = Mem.ullAvailVirtual;
}

//int UpdateCpuUsage()
//{
//	int nCpuUsage = 0;
//	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
//	SYSTEM_TIME_INFORMATION SysTimeInfo;
//	SYSTEM_BASIC_INFORMATION SysBaseInfo;
//
//	double dbIdleTime = 0;
//	double dbSystemTime = 0;
//	LONG status;
//
//	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle(TEXT("ntdll")), "NtQuerySystemInformation");
//
//	if (!NtQuerySystemInformation)
//		return nCpuUsage;
//
//	status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
//	if (status != NO_ERROR)
//		return nCpuUsage;
//
//	status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), 0);
//	if (status != NO_ERROR)
//		return nCpuUsage;
//
//	status = NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
//	if (status != NO_ERROR)
//		return nCpuUsage;
//
//	if (liOldIdleTime.QuadPart != 0){
//		double dbIdleTime1 = 0.0;
//		dbIdleTime1 = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
//		dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
//		dbIdleTime = dbIdleTime1 / dbSystemTime;
//		dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
//	}
//
//	liOldIdleTime = SysPerfInfo.liIdleTime;
//	liOldSystemTime = SysTimeInfo.liKeSystemTime;
//	nCpuUsage = int(dbIdleTime);
//
//	return nCpuUsage;
//}



void GetBaseInfo::onStart()
{	
	//获取当前系统的内存使用率
	MEMORYSTATUSEX   Mem;
	ZeroMemory(&Mem, sizeof(MEMORYSTATUSEX));
	Mem.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&Mem);
	ui.m_TextSysMemUsage->setText(QObject::tr("%1%").arg(Mem.dwMemoryLoad));
	ui.m_TextSysPhysMem->setText(QObject::tr("%1G").arg(Mem.ullTotalPhys/1024/1024/1024));
	ui.m_TextSysVirtMem->setText(QObject::tr("%1G").arg(Mem.ullTotalVirtual/1024/1024/1024));
	ui.m_TextAvailPhysMem->setText(QObject::tr("%1M").arg(Mem.ullAvailPhys / 1024 / 1024 ));
	ui.m_TextAvailVirtMem->setText(QObject::tr("%1M").arg(Mem.ullAvailVirtual / 1024 / 1024));

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(PROCESS_MEMORY_COUNTERS));
	ui.m_TextAppPrivateMem->setText(QObject::tr("%1M").arg(pmc.PagefileUsage / 1024 / 1024));

	//DWORDLONG dwSize = (DWORDLONG)Mem.ullTotalPhys;
	//DWORDLONG dwVirtSize = (DWORDLONG)Mem.ullTotalVirtual;
	//dwAviPhyMBSize = Mem.ullAvailPhys;
	//dwAviVirMBSize = Mem.ullAvailVirtual;

	//ui.m_TextCpuUsage->setText(QObject::tr("%1%").arg(UpdateCpuUsage()));
	/*DWORDLONG dwMemoryLoad = 0;
	DWORDLONG dwSysPhySize = 0;
	DWORDLONG dwSysVirSize = 0;
	GetMemoryInfo(dwMemoryLoad, dwSysPhySize, dwSysVirSize);
	ui.m_TextAllMem->setText("100");
	ui.m_TextCpuNum->setText("10");*/
	return;
}

void GetBaseInfo::onCancel()
{

}
