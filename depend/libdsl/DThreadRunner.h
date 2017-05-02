/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2011-09
 *
 * DThreadRunner.h : 线程运行对象和Heartbeat对象
 */

#ifndef	INCLUDED_LIBDSL_DTHREADRUNNER_H
#define	INCLUDED_LIBDSL_DTHREADRUNNER_H

#include <libdsl/DMutex.h>
#include "DRefObj.h"

#include <deque>
#include <map>

// BEGIN -- 跨平台相关的定义，使用者可以跳过
#ifdef _WIN32

#include <process.h>

#define DSL_THREAD_EXIT_CODE_TYPE unsigned int
#define DSL_THREAD_ENTRY __stdcall
#define DSL_HTHREAD		HANDLE

#define DSL_SLEEP_MS( x )	::Sleep( x )

#define DSL_THREAD_START( thread, start_func, param) \
	( (thread = (DSL_HTHREAD)_beginthreadex(NULL, 0, start_func, param, 0, NULL)) != 0 )

#define DSL_THREAD_END( code )	_endthreadex( code )
#define DSL_THREAD_WAIT_CLOSE( thread ) \
	do { \
		WaitForSingleObject( thread, INFINITE ); \
		CloseHandle( thread ); \
	} while( 0 )

#else

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
// #include <sys/prctl.h>

#define DSL_THREAD_EXIT_CODE_TYPE void *
#define DSL_THREAD_ENTRY 
#define DSL_HTHREAD		pthread_t

#define DSL_SLEEP_MS( x )	\
	do { \
		struct timeval tv; \
		tv.tv_sec = (x) / 1000; \
		tv.tv_usec = ( (x) % 1000 ) * 1000; \
		/* TODO : will EINTR affect ? */ \
		select( 0, NULL, NULL, NULL, &tv ); \
	} while( 0 )

#define DSL_THREAD_START( thread, start_func, param) \
	( pthread_create( &thread, NULL, start_func, param) == 0 )

#define DSL_THREAD_END( code )
#define DSL_THREAD_WAIT_CLOSE( thread ) \
	do { \
		void * tmp = NULL; \
		pthread_join( thread, &tmp ); \
	} while( 0 )

#endif

// END -- 跨平台相关的定义，使用者可以跳过


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// <1> 线程和Heartbeat运行对象，

// DThread 和 DRunner 封装了线程，每个对象封装一个线程，
// DThread 使用继承，子类通过继承，重载run或者heartbeat实现线程函数
// DRunner 使用组合委托，将线程函数作为参数委托给其运行

// 线程退出时，调用 Stop 必须在外部调用，不能在线程内部调用。
// 另外即使线程主动退出，外部也要调用 Stop ，以便回收线程资源。

// run 和 heartbeat 是两种线程运行方式
// run 只运行一次， run 函数退出时，线程就退出
// heartbeat 会被运行多次，在一个循环里不停调用，返回正数会继续运行，返回0会sleep 10ms，返回负数会中止线程

// TODO : 是否合并run和heartbeat成为runloop，将run看成只运行一次就返回负数的heartbeat？？

// TODO : 是否支持起多个线程？

// TODO : 增加priority设置

enum dsl_thread_state_e {
	DSL_THREAD_STATE_UNKNOWN = 0,  // 不应该出现的状态
	DSL_THREAD_STATE_IDLE,         // 线程未启动或者已经停止
	DSL_THREAD_STATE_RUNNING,      // 线程正在运行
	DSL_THREAD_STATE_STOPPING,     // 线程等待退出，等待回收资源（回收后返回IDLE状态）
};

class LIBDSL_API DThread
{
public:
	DThread();
	virtual ~DThread();

	// 启动线程
	virtual int Start();
	// 通知线程退出，
	virtual int SignalStop();
	// 停止线程，并且等待线程退出
	virtual int Stop();

	static void SleepMs( int ms );

	inline bool IsRunning() { return m_state == DSL_THREAD_STATE_RUNNING; }

	inline bool IsStopping() { return m_state == DSL_THREAD_STATE_STOPPING; }
	inline bool IsIdle() { return m_state == DSL_THREAD_STATE_IDLE; }
	void SetThreadName(const char * name); // 设置线程名称 必须在Start前调用

protected:
	virtual int run()
	{
		while( IsRunning() ) {
			int ret = heartbeat();
			if( ret < 0 ) {
				m_state = DSL_THREAD_STATE_STOPPING;
				return ret;
			}
			if( ret == 0 )
				SleepMs( 10 );
		}
		return 0;
	}
	virtual int heartbeat() { return 0; }

	static DSL_THREAD_EXIT_CODE_TYPE DSL_THREAD_ENTRY s_ThreadProc( void * param );

	void SetThreadNameInside(const char * name);

protected:
	DSL_HTHREAD m_thread;
	volatile dsl_thread_state_e m_state;
	char	m_threadName[16]; //
};


// 当使用DSL_RUNNER_TYPE_ONCE类型的runner时，需要在线程函数中定期检查线程是否需要退出，或者使用其他途径通知

enum dsl_runner_type_e {
	DSL_RUNNER_TYPE_UNKNOWN = 0,
	DSL_RUNNER_TYPE_ONCE,
	DSL_RUNNER_TYPE_RUN = DSL_RUNNER_TYPE_ONCE,
	DSL_RUNNER_TYPE_REPEAT,
	DSL_RUNNER_TYPE_HEARTBEAT = DSL_RUNNER_TYPE_REPEAT,
};

template< class T >
class DRunner : public DThread
{
public:
	typedef int ( T::* func_type_t )( DThread * th );
	DRunner( func_type_t func = NULL, T * obj = NULL, dsl_runner_type_e type = DSL_RUNNER_TYPE_ONCE )
		: m_func( func ), m_obj( obj ), m_type( type ) { }

	inline int SetRunner( func_type_t func, T * obj, dsl_runner_type_e type = DSL_RUNNER_TYPE_ONCE )
	{
		m_func = func;
		m_obj = obj;
		m_type = type;
		return 0;
	}

protected:
	virtual inline int run()
	{
		if( m_func == NULL )
			return -1;

		if( m_type == DSL_RUNNER_TYPE_ONCE ) {
			return (m_obj->*m_func)( this );
		}
		else if( m_type == DSL_RUNNER_TYPE_REPEAT ) {
			return DThread::run();
		}
		return -1;
	}
	virtual inline int heartbeat() { return (m_obj->*m_func)( this ); }

protected:
	func_type_t m_func;
	T * m_obj;
	dsl_runner_type_e m_type;
};

typedef int ( * dsl_runner_func_t )( void * arg, void * th );  // th 实际是DThread类型
extern "C" int dsl_runner_is_running( void * th );  // 线程函数中可以调用此函数来判断是否还可以继续运行线程。

template<>
class LIBDSL_API DRunner<void> : public DThread
{
public:
	DRunner( dsl_runner_func_t func = NULL, void * arg = NULL, dsl_runner_type_e type = DSL_RUNNER_TYPE_ONCE )
		: m_func( func ), m_arg( arg ), m_type( type ) { }

	inline int SetRunner( dsl_runner_func_t func, void * arg = NULL, dsl_runner_type_e type = DSL_RUNNER_TYPE_ONCE )
	{
		m_func = func;
		m_arg = arg;
		m_type = type;
		return 0;
	}

protected:
	virtual int run()
	{
		if( m_func == NULL )
			return -1;

		if( m_type == DSL_RUNNER_TYPE_ONCE ) {
			return m_func( m_arg, this );
		}
		else if( m_type == DSL_RUNNER_TYPE_REPEAT ) {
			return DThread::run();
		}
		return -1;
	}
	virtual int heartbeat(); // { return m_func( m_arg ); }

protected:
	dsl_runner_func_t m_func;
	void * m_arg;
	dsl_runner_type_e m_type;
};

// template<> 特化时不需要写这个
inline int DRunner<void>::heartbeat()
{
	return m_func( m_arg, this );
}

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DTHREADRUNNER_H
