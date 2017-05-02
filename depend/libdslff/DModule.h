#ifndef INCLUDED_LIBDSL_MODULE_H
#define INCLUDED_LIBDSL_MODULE_H

#include "libdsl/DMessageQueue.h"
#include "libdsl/DTimerMgr.h"


BEGIN_NAMESPACE_DSL
////////////////////////////////////////////////////////////////////////////////

class DModule;
class DModuleMessage : public DMessage
{
public:
	DModuleMessage(DModule * mdl)
	{
		m_deqSourceModules.push_back(mdl);
	}

private:
	friend class DModule;
	bool m_isAck;
	std::deque< DRefPointer<DModule> > m_deqSourceModules;
};


class DModule : public DMessageQueue, public DTimerHandler
{
public:
	DModule(unsigned int nThreadNum = 1,unsigned int nMaximumQueue = 1024);

	~DModule();

	// 内部消息的通用处理

	// 1  发送消息给目的模块
	int SendMessage(DModuleMessage * pMsg, DModule * dstMdl);
	// 2  发送消息给目的模块 无需返回
	int SendReport(DModuleMessage * pMsg, DModule * dstMdl);
	// 3  回复消息给源模块
	int ReplyMessage(DModuleMessage * pMsg);

	// 定时器操作

	// 1 创建定时时 返回值为timerID 返回-1表示失败
	unsigned int CreateTimer();
	// 2 启动定时器 返回0表示成功 其他表示失败
	int StartTimer(unsigned int timerID, int timeout_ms);
	// 3 关闭定时器 返回0表示成功 其他表示失败
	int CloseTimer(unsigned int timerID);


public:
	virtual void OnMessage(DModuleMessage* pMsg);				// 收到其他模块的消息请求
	virtual void OnReplyMessage(DModuleMessage * pMsg);			// 收到目标模块的消息回复
	virtual void OnTimeout( unsigned int timer_id );	// 定时器到期

protected:
	virtual void processMsg( DMessage * pMsg );

	DTimerMgr m_TimerMgr;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_MODULE_H