/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2011-09
 *
 * DTimerMgr.h : 定时器处理
 */

#ifndef INCLUDED_LIBDSL_DTIMERMGR_H
#define INCLUDED_LIBDSL_DTIMERMGR_H

#include <libdsl/dslbase.h>
#include <libdsl/DRefObj.h>
#include <libdsl/DThreadRunner.h>
#include <libdsl/DEvent.h>

#include <list>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

#define INVALID_TIMER_ID	((unsigned int)-1)

class LIBDSL_API DTimerHandler : virtual public DRefObj
{
public:
	DTimerHandler() {}
	DSL_DEPRECATED DTimerHandler( DRefObj * obj ) {}

	virtual void OnTimeout( unsigned int timer_id ) = 0;

protected:
	virtual ~DTimerHandler() {}
};

// TODO : 暂时先单线程回调，以后再考虑多线程回调timer，到时要注意同一个Handler的多个timer回调的串行化
//        如果timer数量较多，临时解决方案可以通过创建多个DTimerMgr对象来利用多线程

// Timer都是定期执行的，如果不想执行，则调用StopTimer()

class LIBDSL_API DTimerMgr
{
public:
	DTimerMgr();
	virtual ~DTimerMgr();

	// create timer, and start, ret_timer_id will get id value before OnTimeout callback
	void CreateTimer( DTimerHandler * handler, int timeout_ms, unsigned int * ret_timer_id );
	// create timer, not start, return timer id
	unsigned int CreateTimer( DTimerHandler * handler );
	int StartTimer( unsigned int timer_id, int timeout_ms );
	int PauseTimer( unsigned int timer_id );
	DSL_DEPRECATED int StopTimer( unsigned int timer_id ) { return PauseTimer( timer_id ); }
	int DestroyTimer( unsigned int timer_id );
	DSL_DEPRECATED int CloseTimer( unsigned int timer_id ) { return DestroyTimer( timer_id ); }

	inline int GetTimerNum() { m_mtxTimers.Lock(); int ret = (int)m_Timers.size(); m_mtxTimers.Unlock(); return ret; }

protected:
	int runTimer( DThread * th );

protected:
	class TNode
	{
	public:
		unsigned int m_id;
		DRef<DTimerHandler> m_handler;
		uint32_t m_next_tick;
		int timeout_ms;
	};

	unsigned int m_next_id;       // 下一个timer id，作为hint，TODO：需优化
	DMutex m_mtxTimers;
	std::list< TNode > m_Timers;  // 按时间排序的正在运行的timer
	std::map<int, TNode> m_preTimers;  // 未运行或停止的timer

	DEvent m_evt;
	DRunner<DTimerMgr> m_runner;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DTIMERMGR_H
