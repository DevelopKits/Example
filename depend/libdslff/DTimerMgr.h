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

// TODO : ��ʱ�ȵ��̻߳ص����Ժ��ٿ��Ƕ��̻߳ص�timer����ʱҪע��ͬһ��Handler�Ķ��timer�ص��Ĵ��л�
//        ���timer�����϶࣬��ʱ�����������ͨ���������DTimerMgr���������ö��߳�

// Timer���Ƕ���ִ�еģ��������ִ�У������StopTimer()

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

	unsigned int m_next_id;       // ��һ��timer id����Ϊhint��TODO�����Ż�
	DMutex m_mtxTimers;
	std::list< TNode > m_Timers;  // ��ʱ��������������е�timer
	std::map<int, TNode> m_preTimers;  // δ���л�ֹͣ��timer

	DEvent m_evt;
	DRunner<DTimerMgr> m_runner;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DTIMERMGR_H
