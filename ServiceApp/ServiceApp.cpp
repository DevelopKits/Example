// ServiceApp.cpp : 定义控制台应用程序的入口点。
//

#include <windows.h>
#include "stdafx.h"
#include "MyLogger.h"

MyLogger * pMyLogger = NULL;

static SERVICE_STATUS        sAppstatus;          ///< 服务状态
static SERVICE_STATUS_HANDLE shAppHandle;   ///< 服务状态句柄

SC_HANDLE                    hSCM = NULL; ///< 服务管理列表
SC_HANDLE                    hIVMSService = NULL; ///< 服务进程句柄

void WINAPI ServiceCtrlHandler(DWORD dwControl)
{
	switch (dwControl)
	{
	case SERVICE_CONTROL_PAUSE:
		sAppstatus.dwCurrentState = SERVICE_PAUSE_PENDING;
		::SetServiceStatus(shAppHandle, &sAppstatus);
		break;

	case SERVICE_CONTROL_CONTINUE:
		sAppstatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
		::SetServiceStatus(shAppHandle, &sAppstatus);
		sAppstatus.dwCurrentState = SERVICE_RUNNING;
		break;

	case SERVICE_CONTROL_STOP:
		sAppstatus.dwCurrentState = SERVICE_STOP_PENDING;
		::SetServiceStatus(shAppHandle, &sAppstatus);
		sAppstatus.dwCurrentState = SERVICE_STOPPED;
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		exit(1);
		break;

	case SERVICE_CONTROL_INTERROGATE:
		sAppstatus.dwCurrentState = SERVICE_RUNNING;
		break;
	}

	::SetServiceStatus(shAppHandle, &sAppstatus);
}

BOOL Stop()
{
	DEBUG_LOG("main stop server");
	if (hSCM == NULL)
	{
		hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	}

	// open the service 
	if (hIVMSService == NULL)
	{
		hIVMSService = OpenServiceA(hSCM,
			"ServiceAPP",     //rename the service name with you service name 
			SERVICE_ALL_ACCESS);
	}

	if ((hSCM == NULL) || (hIVMSService == NULL))
	{
		return FALSE;
	}

	SERVICE_STATUS sServicStatus;
	BOOL bRet = ControlService(hIVMSService,
		SERVICE_CONTROL_STOP,
		&sServicStatus);
	if (!bRet)
	{
		GetLastError();
	}

	CloseServiceHandle(hSCM);
	hSCM = NULL;

	return   TRUE;
}

void WINAPI ServiceStatus(DWORD argc, LPTSTR * argv)
{
	DEBUG_LOG("ServiceStatus start");
	sAppstatus.dwServiceType = SERVICE_WIN32;
	sAppstatus.dwCurrentState = SERVICE_START_PENDING;
	sAppstatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	sAppstatus.dwWin32ExitCode = 0;
	sAppstatus.dwServiceSpecificExitCode = 0;
	sAppstatus.dwCheckPoint = 0;
	sAppstatus.dwWaitHint = 0;

	shAppHandle = ::RegisterServiceCtrlHandler((TCHAR*)"ServiceAPP", ServiceCtrlHandler);
	if (shAppHandle == (SERVICE_STATUS_HANDLE)0)
	{
		DEBUG_LOG("RegisterServiceCtrlHandler failed\n");
		return;
	}

	sAppstatus.dwCheckPoint = 0;
	sAppstatus.dwWaitHint = 0;

	sAppstatus.dwCurrentState = SERVICE_RUNNING;
	::SetServiceStatus(shAppHandle, &sAppstatus);

//while (1)
//{
//	DEBUG_LOG("ServiceStatus working");
//	Sleep(1000);
//}
	return;
}

void InstallService(char * szServiceName)
{
	SC_HANDLE handle = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	char szFilename[256];
	::GetModuleFileNameA(NULL, szFilename, 255);
	SC_HANDLE hService = ::CreateServiceA(handle, szServiceName,
		szServiceName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, szFilename, NULL,
		NULL, NULL, NULL, NULL);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(handle);
}

int main(int argc, char* argv[])
{
	char strExePathTmp[260] = { 0 };
	GetModuleFileNameA(NULL, strExePathTmp, 260);
	std::string strExePath(strExePathTmp);
	strExePath.resize(strExePath.find_last_of('\\'));
	SetCurrentDirectoryA(strExePath.c_str());

	pMyLogger = MyLogger::getInstance();
	if ((argc == 2) && (::strcmp((const char*)(argv[1]), "install") == 0))
	{
		DEBUG_LOG("install service" );
		InstallService("ServiceAPP");

		return 0;
	}

	SERVICE_TABLE_ENTRY   service_table_entry[] =
	{
		{ (LPWSTR)("ServiceApp"), ServiceStatus },
		{ NULL, NULL }
	};

	BOOL bRet = ::StartServiceCtrlDispatcher(service_table_entry);
	if (!bRet)
	{
		ERROR_LOG("StartServiceCtrlDispatcher failed");
	}
	return 0;
}

