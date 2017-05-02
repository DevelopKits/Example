/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* DTrader.h : ����Ա
* ����    ��������
* ������ڣ�2014��10��10��
*
* ��ǰ�汾��1.0
*/

#pragma once


#include <cwchar>
#include <string>
#include <stdio.h>

#include <libdsl/DHttpClient.h>
#include <libdsl/DTimerMgr.h>
#include <libdsl/ESB/DAutoPtr.h>


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

#define Trader_Timeout		30

class DMsgBus;
class LIBDSL_API DTrader : public DHttpHandler , public DTimerHandler
{
public:
	DTrader(DMsgBus* pBus, int nTimeout = Trader_Timeout);
	virtual ~DTrader() {}

	int Connect(const char * ip, int port, DHttpHandler* pClientHandler);
	void Close();

	void ReConnect(int nTime = 10);				// nTime�������ʼ������ ���ڶ���������һ����OnClose()�¼���ʹ�á�
	void SetTimeout(int64_t n64Timeout);		// Ĭ��20�룬��λΪ �롣 ���Զ�̬�ı䡣
	bool IsTimeout();

	int GetTraderIdInt(){ return m_nUnqSign; }
	const char* GetTraderId(){ return m_sTraderId.c_str(); }

	// ���ݷ��ͽӿ�
	int Send( const char* szBuf, int nLen );
	int SendHttp( const DHttp & httpmsg );
	// return 1 : got msg, 0 : timeout, < 0 : error
	int WaitHttp( DHttp * httpmsg, int timeout_ms );

	// �����Ϊclient����Ҫ���øú�����  
	// �����Ϊ����˵Ĵ��������Ҫ���øú�����
	void SetTrader(DRef<DHttpSession> pHttpSession, const char* szIp, int nPort, int nUnqSign, DHttpHandler* pHandler);

protected:
	//// DTrader �����¼�
	virtual int OnReconnect() { return 0; }
	//// DTrader

	//// DTimerHandler ���棺�������ظú�������Ҫ�ֶ����û���ĸ÷�����
	virtual void OnTimeout( unsigned int timer_id );
	//// DTimerHandler

protected:
	//// DHttpHandler  ���棺�������ظú�������Ҫ�ֶ����û���ĸ÷�����
	virtual int OnConnect( const DRef<DHttpSession> & httpsess, const char * remote_ip, int remote_port );
	virtual int OnHttp( const DRef<DHttpSession> & httpsess, const DHttp & httpmsg  );
	virtual int OnClose( const DRef<DHttpSession> & httpsess );
	//// DHttpHandler	 

private:
	DHttpHandler*		m_pHandler;
	DMutex				m_mtxLock;
	std::string			m_sIp;
	int					m_nPort;
	std::string			m_sTraderId;
	int64_t				m_tTimeout;
	int64_t				m_tPreTime;
	int					m_nUnqSign;

	DAutoPtr<DHttpClient> m_pHttpClient;
	unsigned int		m_timerId;
	DMsgBus*			m_pBus;
	DRef<DHttpSession>	m_pHttpSession;
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL