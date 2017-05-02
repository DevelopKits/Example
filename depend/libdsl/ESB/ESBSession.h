/*
* All rights reserved.
*
* �ļ����ƣ�ESBSession.h
* �ļ���ʶ��
* ժ����Ҫ��ͨ�ûỰ����
*

* ��ǰ�汾��1.0
* ԭ���ߡ���������
* ������ڣ�2014��11��7��
* �޶���¼������
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
	 * ʹ��ǰ�����Ȱ� bus��
	 * pTimerHandler�� NULL ��ʾ����̳�ESBSession��������OnTimeout()
	***/
	void Bind(DMsgBus* pBus, DTimerHandler* pTimerHandler = NULL) { m_pBus = pBus; m_pTimerHandler = pTimerHandler; } 
	
	/***
	 * ��������
	 * tId: key
	 * tSession: value
	 * nTime: ��ʱʱ�䣬Ĭ��20��
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
	 * ɾ�����ݡ�
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
	 * ȡ�����ݣ����������б���ɾ����
	 * tId: ��Ӧsession��keyֵ
	 * timer_id: OnTimeout �еĳ�ʱID
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