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

class DTimerHandler : public DRefInterface
{
public:
	DTimerHandler( DRefObj * refObj ) : DRefInterface( refObj ) {}
	virtual ~DTimerHandler() {}

	virtual void OnTimeout( unsigned int timer_id ) = 0;
};

// TODO : 暂时先单线程回调，以后再考虑多线程回调timer，到时要注意同一个Handler的多个timer回调的串行化
//        如果timer数量较多，临时解决方案可以通过创建多个DTimerMgr对象来利用多线程

// Timer都是定期执行的，如果不想执行，则调用StopTimer()

class DTimerMgr
{
public:
	DTimerMgr();
	virtual ~DTimerMgr();

	unsigned int CreateTimer(DTimerHandler * handler);
	int StartTimer( unsigned int timer_id, int timeout_ms );
	int StopTimer( unsigned int timer_id );
	int CloseTimer( unsigned int timer_id );

	int GetTimerNum() { m_mtxTimers.Lock(); int ret = (int)m_Timers.size(); m_mtxTimers.Unlock(); return ret; }

protected:
	int runTimer( DThread * th );

protected:
	class TNode
	{
	public:
		unsigned int m_id;
		DRefInterfacePointer<DTimerHandler> m_handler;
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
