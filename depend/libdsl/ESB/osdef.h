// osdef.h

#ifndef	__OSDEF_H_
#define	__OSDEF_H_

#if	defined(_WIN32) || defined(WIN32) || defined(__WIN32__)

#	include "win32.h"

#elif	defined(LINUX)  || defined(_LINUX) || defined(__linux__) 

#	include	"linux.h"

#else

#	error	Missing <osdef.h> support for this platform.

#endif

#endif	//__OSDEF_H_
