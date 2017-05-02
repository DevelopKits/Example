/*
* All rights reserved.
*
* 文件名称：ESBSession.h
* 文件标识：
* 摘　　要：通用会话处理
*

* 当前版本：1.0
* 原作者　：吕少卿
* 完成日期：2014年11月7日
* 修订记录：创建
*/
#pragma once

#include <libdsl/DTimerMgr.h>
#include <libdsl/ESB/ISession.h>


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class DMsgBus;
template<class SESSION_ID, class SESSION>
class ESBSession : public DTimerHandler
{
public:
	ESBSession() { m_pBus = NULL; }

	/***
	 * 使用前必须先绑定 bus。
	 * pTimerHandler： NULL 表示子类继承ESBSession，并重载OnTimeout()
	***/
	void Bind(DMsgBus* pBus, DTimerHandler* pTimerHandler = NULL) { m_pBus = pBus; m_pTimerHandler = pTimerHandler; } 
	
	/***
	 * 增加数据
	 * tId: key
	 * tSession: value
	 * nTime: 超时时间，默认20秒
	***/
	int AddSession(SESSION_ID tId, SESSION tSession, unsigned int nTime = 20)
	{
		unsigned int nTimer = m_pBus->SetTimer(nTime * 1000, this);
		if(nTimer < 0)
			return nTimer;

		if(m_sessTimerKey.AddSession(nTimer, tId) != 0)
			return -1;

		if(m_sessKeyTimer.AddSession(tId, nTimer) != 0)
			return -2;

		if(m_sessKeyValue.AddSession(tId, tSession) != 0)
			return -3;

		return 0;
	}

	/***
	 * 删除数据。
	***/
	void DelSession(SESSION_ID tId)
	{
		unsigned int timer_id;
		if(m_sessKeyTimer.FetchSession(tId, timer_id) != 0)
			return;

		m_pBus->KillTimer(timer_id);
		m_sessTimerKey.DelSession(timer_id);
		m_sessKeyValue.DelSession(tId);
	}

	/***
	 * 取走数据，该数据在列表中删除。
	 * tId: 对应session的key值
	 * timer_id: OnTimeout 中的超时ID
	***/
	int FetchSessionBySId( SESSION_ID tId, SESSION &tSession)
	{
		unsigned int timer_id;
		if(m_sessKeyTimer.FetchSession(tId, timer_id) != 0)
			return -1;

		m_pBus->KillTimer(timer_id);
		m_sessKeyTimer.DelSession(tId);

		if(m_sessKeyValue.FetchSession(tId, tSession) != 0)
			return -2;

		return 0;
	}
	int FetchSessionByTId( unsigned int timer_id, SESSION &tSession)
	{
		SESSION_ID sId;
		if(m_sessTimerKey.FetchSession(timer_id, sId) != 0)
			return -1;

		m_pBus->KillTimer(timer_id);
		m_sessKeyTimer.DelSession(sId);

		if(m_sessKeyValue.FetchSession(sId, tSession) != 0)
			return -2;

		return 0;
	}

	//// DTimerHandler
	virtual void OnTimeout( unsigned int timer_id )
	{
		if(m_pTimerHandler.GetPointer())
			m_pTimerHandler->OnTimeout(timer_id);
	}
	//// DTimerHandler

protected:
	virtual ~ESBSession() {}

private:
	ISession<SESSION_ID, SESSION>		m_sessKeyValue;
	ISession<unsigned int, SESSION_ID>	m_sessTimerKey;
	ISession<SESSION_ID, unsigned int>	m_sessKeyTimer;
	DMsgBus* m_pBus;
	DRef<DTimerHandler> m_pTimerHandler;
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL