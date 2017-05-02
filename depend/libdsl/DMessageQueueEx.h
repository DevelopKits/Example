/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
* 2011-09
*
* DMessageQueueEx.h : 带优先级消息队列对象
*/

#ifndef	INCLUDED_LIBDSL_DMESSAGEQUEUEEX_H
#define	INCLUDED_LIBDSL_DMESSAGEQUEUEEX_H

#include <libdsl/DMessageQueue.h>
#include <libdsl/DRefObj.h>
#include <libdsl/DThreadRunner.h>
#include <libdsl/DEvent.h>
#include <deque>
#include <vector>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////
typedef enum _EnumQueueLevel_
{
	queue_high  = 0,	// 高优先级
	queue_normal = 1,	// 普通优先级
	queue_low = 2,		// 低优先级
	queue_max,
}EnumQueueLevel;

class DMessageData;

class LIBDSL_API DMessageQueueEx : virtual public DRefObj
{
public:
	DMessageQueueEx(unsigned int nThreadNum = 1,unsigned int nMaximumQueue = 1024) ;
	
	virtual int Start();
	virtual int SignalStop();
	virtual int Stop();

	virtual int PushMsg(DMessage* pMsg, EnumQueueLevel level = queue_normal);
	virtual int QueueSize();

	// 外部驱动消息列队,此时不需要调用 Start SignalStop Stop等线程操作函数
	// 返回值 0 空闲 1 繁忙
	virtual int HeartBeat(); 

	// 清理消息列队中未处理消息
	// 返回值 0 成功 -1 失败
	int		ClearMsg(EnumQueueLevel level = queue_normal);


	bool IsRunning(void) const ;


	void SetThreadName(const char * name); // 设置线程名称 最长15字节 必须在Start前调用

protected:
	virtual ~DMessageQueueEx();
	virtual void processMsg( DMessage * pMsg ) = 0;
	int driver_engine( DThread * th );

protected:

	DMessageData * m_impl;

};

// 消息队列模板，封装消息处理函数的注册和分派
// 使用方法 class MyMessageQueue : public DMessageQueueTpl<MyMessageQueue> { ... }; 
// 然后在构造函数或初始化函数中调用 AddFunction() 注册消息处理函数

template< class T >
class DMessageQueueTplEx : public DMessageQueueEx
{
public:
	DMessageQueueTplEx( unsigned int nThreadNum = 1,unsigned int nMaximumQueue = 1024 )
		: DMessageQueueEx( nThreadNum, nMaximumQueue )
	{
	}

protected:
	virtual ~DMessageQueueTplEx() {}
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

#endif 
