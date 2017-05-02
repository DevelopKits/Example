/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2011-09
 *
 * DEvent.h : 事件对象
 */

#ifndef INCLUDED_LIBDSL_DEVENT_H
#define INCLUDED_LIBDSL_DEVENT_H

#include <libdsl/dslbase.h>
#include <libdsl/DMutex.h>

// BEGIN -- 跨平台相关的定义，使用者可以跳过

#ifdef _WIN32
#define DSL_EVENT_CS(evt)			HANDLE evt
#define	DSL_EVENT_csConstruct(evt)	(evt = ::CreateEvent(NULL,FALSE,FALSE,NULL))
#define	DSL_EVENT_csDestruct(evt)	::CloseHandle(evt)
#define	DSL_EVENT_csSet(evt)		::SetEvent(evt)
#define DSL_EVENT_csReset(evt)		::ResetEvent(evt)
#define	DSL_EVENT_csWait(evt,lck,timeout_ms,result)	\
	do { \
		lck.Unlock(); \
		DWORD t = (timeout_ms == -1) ? INFINITE : timeout_ms; \
		DWORD tmp = ::WaitForSingleObject(evt,t); \
		result = (tmp == WAIT_OBJECT_0) ? 1 : 0; \
		lck.Lock(); \
	} while( 0 )

#else

#include <pthread.h>
#include <time.h>

#ifdef MAC 
#include <mach/clock.h> 
#include <mach/mach.h> 
#endif 

#define DSL_EVENT_CS(evt)			pthread_cond_t evt
#define	DSL_EVENT_csConstruct(evt)	pthread_cond_init(&evt, NULL)
#define	DSL_EVENT_csDestruct(evt)	pthread_cond_destroy(&evt)
#define	DSL_EVENT_csSet(evt)		pthread_cond_signal(&evt)
#define DSL_EVENT_csReset(evt)		do {} while(0)
#ifdef MAC
#define	DSL_EVENT_csWait(evt,lck,timeout_ms,result)	\
	do { \
	int tmp = 0; \
	if( timeout_ms == -1 ) { \
	tmp = pthread_cond_wait(&evt, lck.m_csLock); \
	} \
		else { \
struct timespec t; \
	clock_serv_t cclock; \
	mach_timespec_t mts; \
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);\
	clock_get_time(cclock, &mts); \
	mach_port_deallocate(mach_task_self(), cclock); \
	t.tv_sec = mts.tv_sec;\
	t.tv_nsec = mts.tv_nsec;\
	t.tv_sec += timeout_ms / 1000; \
	t.tv_nsec += (timeout_ms % 1000) * 1000 * 1000; \
	if( t.tv_nsec >= 1000 * 1000 * 1000 ) { \
	t.tv_sec += 1; \
	t.tv_nsec -= 1000 * 1000 * 1000; \
	} \
	tmp = pthread_cond_timedwait(&evt,lck.m_csLock,&t); \
		} \
		result = (tmp == 0) ? 1 : 0; \
	} while( 0 )

#else
#define	DSL_EVENT_csWait(evt,lck,timeout_ms,result)	\
	do { \
	int tmp = 0; \
	if( timeout_ms == -1 ) { \
	tmp = pthread_cond_wait(&evt, lck.m_csLock); \
	} \
		else { \
struct timespec t; \
	clock_gettime(CLOCK_REALTIME, &t);\
	t.tv_sec += timeout_ms / 1000; \
	t.tv_nsec += (timeout_ms % 1000) * 1000 * 1000; \
	if( t.tv_nsec >= 1000 * 1000 * 1000 ) { \
	t.tv_sec += 1; \
	t.tv_nsec -= 1000 * 1000 * 1000; \
	} \
	tmp = pthread_cond_timedwait(&evt,lck.m_csLock,&t); \
		} \
		result = (tmp == 0) ? 1 : 0; \
	} while( 0 )

#endif






#endif

// END -- 跨平台相关的定义，使用者可以跳过

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// <1> 线程锁定义

class LIBDSL_API DEvent
{
public:
	DEvent(void);
	~DEvent(void);

	inline int SetEvent() { Lock(); int ret = SetEventInLock(); Unlock(); return ret; }
	int Reset();

	inline int Wait() { return TimedWait( -1 ); }
	inline int TimedWait( int timeout_ms ) { Lock(); int ret = TimedWaitInLock(timeout_ms); Unlock(); return ret; }

	inline void Lock() { m_lock.Lock(); return; }
	inline void Unlock() { m_lock.Unlock(); return; }

	int SetEventInLock();
	// 等待，直到有事件或者超时，调用函数前要先加锁，内部等待过程中会解锁，函数返回前会重新加锁
	// ret 0 : timeout, 1 : get event;
	int TimedWaitInLock( int timeout_ms );

private: // 不可拷贝
	DEvent( const DEvent & mtx );
	DEvent & operator = ( const DEvent & mtx );

private:
	DMutex m_lock;
	int m_isset;
	DSL_EVENT_CS(m_evt);
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_DEVENT_H
