/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
* 2011-09
*
* DMessageQueue.h : 消息队列对象
*/

#ifndef	INCLUDED_LIBDSL_DMESSAGEQUEUE_H
#define	INCLUDED_LIBDSL_DMESSAGEQUEUE_H

#include <libdsl/DRefObj.h>
#include <libdsl/DThreadRunner.h>
#include <libdsl/DEvent.h>
#include <deque>
#include <vector>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class LIBDSL_API DMessage : virtual public DRefObj
{
public:
	DMessage(int type = 0) : m_type(type) {}
	virtual ~DMessage() {}

	inline void SetType( int type ) { m_type = type; return; }
	inline int GetType() const { return m_type; }

protected:
	int m_type;
};

class LIBDSL_API DMessageQueue : virtual public DRefObj
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
	void SetThreadName(const char * name); // 设置线程名称 最长15字节 必须在Start前调用

protected:
	virtual ~DMessageQueue();
	virtual void processMsg( DMessage * pMsg ) = 0;

protected:
	unsigned int m_maximumQueue;
	unsigned int m_threadNum;
	
	DEvent m_evt;
	std::deque< DRef<DMessage> > m_dequeMsg;
	
	int driver_engine( DThread * th );
	std::vector< DRunner<DMessageQueue> > m_vecThread;
	bool m_isRunning;

};

// 消息队列模板，封装消息处理函数的注册和分派
// 使用方法 class MyMessageQueue : public DMessageQueueTpl<MyMessageQueue> { ... }; 
// 然后在构造函数或初始化函数中调用 AddFunction() 注册消息处理函数

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
		// 子类可重载，进行不认识的消息的处理，缺省是直接忽略
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

