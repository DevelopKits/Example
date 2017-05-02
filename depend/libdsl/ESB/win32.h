
#ifndef	__WIN32_H__
#define	__WIN32_H__

#if	!defined(_MSC_VER) && (!defined(__BORLANDC__) || __BORLANDC__ < 0x500)
#	error	Microsoft Visual C/C++ or Inprise C++ Builder compatible compiler is needed for <win32.h>.
#endif

#include <Windows.h>
#include <io.h>

#ifndef snprintf
#	define snprintf sprintf_s
#endif

#ifndef sleep
#	define sleep(x) Sleep(x * 1000);
#endif

#define STD_CALL		__stdcall
typedef unsigned long	DSL_THREAD_ID;


/////////////////
// DLL support //
/////////////////
#ifndef dlopen
#define RTLD_LAZY					0
#define RTLD_NOW 					1
#define RTLD_GLOBAL					2
#define HDDLL						HINSTANCE 
#define dlopen(file, flag)			LoadLibrary(file)
#define dlclose(handle)				FreeLibrary(handle)
#define	dlsym(hDll, szSymbol)		GetProcAddress(hDll, szSymbol)

__inline PCHAR dllErrorStr()
{
	PCHAR strError;

	return FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, GetLastError(), 0, (LPTSTR)&strError, sizeof(strError), NULL)
		? strError
		: "";
}
#endif



#endif	// __WIN32_H__
