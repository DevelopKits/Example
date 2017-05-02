#ifndef	INCLUDED_LIBDSL_DMESSAGEQUEUE_H
#define	INCLUDED_LIBDSL_DMESSAGEQUEUE_H

#include <libdsl/DRefObj.h>
#include <libdsl/DThreadRunner.h>
#include <libdsl/DEvent.h>
#include <deque>
#include <vector>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class DMessage : public DRefObj
{
public:
	DMessage(int type = 0) : m_type(type) {}
	virtual ~DMessage() {}

	void SetType( int type ) { m_type = type; return; }
	int GetType() const { return m_type; }

protected:
	int m_type;
};

class DMessageQueue : public DRefObj
{
public:
	DMessageQueue(unsigned int nThreadNum = 1,unsigned int nMaximumQueue = 1024) 
		: m_maximumQueue(nMaximumQueue)
		, m_threadNum(nThreadNum)
		, m_isRunning(false)
	{}

	virtual int Start();
	virtual int SignalStop();
	virtual int Stop();

	virtual int PushMsg( DMessage* pMsg );
	virtual int QueueSize() { return (int)m_dequeMsg.size(); }
	bool IsRunning(void) const { return m_isRunning; }
	void SetThreadName(const char * name); // �����߳����� �15�ֽ� ������Startǰ����

protected:
	virtual ~DMessageQueue();
	virtual void processMsg( DMessage * pMsg ) = 0;

protected:
	unsigned int m_maximumQueue;
	unsigned int m_threadNum;
	
	DEvent m_evt;
	std::deque< DRefPointer<DMessage> > m_dequeMsg;
	
	int driver_engine( DThread * th );
	std::vector< DRunner<DMessageQueue> > m_vecThread;
	bool m_isRunning;

};

// ��Ϣ����ģ�壬��װ��Ϣ��������ע��ͷ���
// ʹ�÷��� class MyMessageQueue : public DMessageQueueTpl<MyMessageQueue> { ... }; 
// Ȼ���ڹ��캯�����ʼ�������е��� AddFunction() ע����Ϣ������

template< class T >
class DMessageQueueTpl : public DMessageQueue
{
public:
	DMessageQueueTpl( unsigned int nThreadNum = 1,unsigned int nMaximumQueue = 1024 )
		: DMessageQueue( nThreadNum, nMaximumQueue )
	{
	}

protected:
	virtual ~DMessageQueueTpl() {}
	typedef void (T::*MsgFunc_t)( DMessage * pMsg );
	virtual void processMsg( DMessage * pMsg )
	{
		typename std::map< int, MsgFunc_t >::iterator it = m_funcMap.find( pMsg->GetType() );
		if( it == m_funcMap.end() )
			OnUnknownMsg( pMsg );
		else
			(((T*)this)->*(it->second))( pMsg );
		return;
	}
	virtual void OnUnknownMsg( DMessage * /*pMsg*/ )
	{
		// ��������أ����в���ʶ����Ϣ�Ĵ���ȱʡ��ֱ�Ӻ���
		return;
	}

protected:
	void AddMsgFunc( int type, MsgFunc_t func )
	{
		m_funcMap[type] = func;
		return;
	}
	void DelMsgFunc( int type )
	{
		typename std::map< int, MsgFunc_t >::iterator it = m_funcMap.find( type );
		if( it != m_funcMap.end() )
			m_funcMap.erase( it );
		return;
	}

protected:
	std::map< int, MsgFunc_t > m_funcMap;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DMESSAGEQUEUE_H
