#ifndef INCLUDED_LIBDSL_DMUTEX_H
#define INCLUDED_LIBDSL_DMUTEX_H

#include <libdsl/dslbase.h>

// BEGIN -- 跨平台相关的定义，使用者可以跳过

// TODO : win32的critical section是递归锁，不能改。
// 但我们不推荐使用递归锁，所以linux下是否还支持递归锁？？windows下要不要加一个递归判断？？

#ifdef _WIN32
#define	DSL_MUTEX_CS(hcs)				CRITICAL_SECTION	hcs[1];
#define	DSL_MUTEX_csConstruct(hcs)	(InitializeCriticalSection(hcs), TRUE)
#define	DSL_MUTEX_csDestruct(hcs)		DeleteCriticalSection(hcs)
// #define	DSL_MUTEX_csTryEnter(hcs)		TryEnterCriticalSection(hcs)
#define	DSL_MUTEX_csEnter(hcs)		EnterCriticalSection(hcs)
#define	DSL_MUTEX_csLeave(hcs)		LeaveCriticalSection(hcs)

#else

#include <pthread.h>

#define	DSL_MUTEX_CS(hcs)					pthread_mutex_t	hcs[1];
#ifdef MAC
#define DSL_MUTEX_csConstruct( hcs ) \
	do { \
	pthread_mutexattr_t attr;  \
	pthread_mutexattr_init( &attr );  \
	/* 设置 recursive */ \
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE);  \
	pthread_mutex_init( hcs, &attr ); \
	pthread_mutexattr_destroy( &attr ); \
	} while( 0 )
#else
#define DSL_MUTEX_csConstruct( hcs ) \
	do { \
	pthread_mutexattr_t attr;  \
	pthread_mutexattr_init( &attr );  \
	/* 设置 recursive */ \
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );  \
	pthread_mutex_init( hcs, &attr ); \
	pthread_mutexattr_destroy( &attr ); \
	} while( 0 )
#endif


#define	DSL_MUTEX_csDestruct(hcs)				pthread_mutex_destroy( hcs )
// #define	DSL_MUTEX_csTryEnter(hcs)				(0 == pthread_mutex_trylock(hcs))
#define	DSL_MUTEX_csEnter(hcs)				pthread_mutex_lock(hcs)
#define	DSL_MUTEX_csLeave(hcs)				pthread_mutex_unlock(hcs)

#endif

// END -- 跨平台相关的定义，使用者可以跳过

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// <1> 线程锁定义
class DEvent;

class DMutex
{
public:
	DMutex(void);
	~DMutex(void);

	void Lock();
	void Unlock();

private: // 不可拷贝
	DMutex( const DMutex & mtx );
	DMutex & operator = ( const DMutex & mtx );

private:
	friend class DEvent;
	DSL_MUTEX_CS(m_csLock);
};

/*	利用生命周期，加锁与解锁，便于使用
 *	DMutexGuard guard(DMutex);
 */
class DMutexGuard
{
public:
	DMutexGuard(DMutex *);
	~DMutexGuard();

private: // 不可拷贝
	DMutexGuard( const DMutexGuard & mtx );
	DMutexGuard & operator = ( const DMutexGuard & mtx );

private:
	DMutex *m_pDMutex;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_DMUTEX_H

