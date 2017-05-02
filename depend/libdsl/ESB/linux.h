
#ifndef	__linux_h__
#define	__linux_h__


#if	!defined(__GNUC__) && (!defined(__INTEL_COMPILER) || !defined(__linux__))
#	error	GNU C/C++ or compatible compiler is needed for <linux.h>.
#endif

// _ISOC99_SOURCE
//	Use ISO C99 extension. If the compiler supports ISO C99, it should define macro __USE_ISOC99 intrinsic
//	or somewhere in <features.h> or <stddef.h>.
#ifndef	_ISOC99_SOURCE
#	define	_ISOC99_SOURCE	1
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>  
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h> 
#include <dirent.h>  

#define STD_CALL

typedef	int					BOOL;
#define	FALSE				0
#define	TRUE				1
typedef pid_t				DSL_THREAD_ID;


#define GetCurrentThreadId()				syscall(SYS_gettid)
#define THREAD_PRIORITY_HIGHEST				99
#define SetThreadPriority(hthread, level)	pthread_setschedprio(hthread, level)
#define _getch				getchar


/////////////////
// DLL support //
/////////////////
#ifndef HDDLL
#define HDDLL				void* 
__inline	const char*	dllErrorStr(void)
{
	const char*	strError = dlerror();
	return strError ? strError : "";
}
#endif 


#endif	// __linux_h__
