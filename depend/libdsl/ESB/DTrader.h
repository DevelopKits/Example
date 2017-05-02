/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* DTrader.h : 交易员
* 作者    ：吕少卿
* 完成日期：2014年10月10日
*
* 当前版本：1.0
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

	void ReConnect(int nTime = 10);				// nTime：几秒后开始重连。 用于断线重连，一般在OnClose()事件中使用。
	void SetTimeout(int64_t n64Timeout);		// 默认20秒，单位为 秒。 可以动态改变。
	bool IsTimeout();

	int GetTraderIdInt(){ return m_nUnqSign; }
	const char* GetTraderId(){ return m_sTraderId.c_str(); }

	// 数据发送接口
	int Send( const char* szBuf, int nLen );
	int SendHttp( const DHttp & httpmsg );
	// return 1 : got msg, 0 : timeout, < 0 : error
	int WaitHttp( DHttp * httpmsg, int timeout_ms );

	// 如果作为client，则不要调用该函数。  
	// 如果作为服务端的存根，则需要调用该函数。
	void SetTrader(DRef<DHttpSession> pHttpSession, const char* szIp, int nPort, int nUnqSign, DHttpHandler* pHandler);

protected:
	//// DTrader 重连事件
	virtual int OnReconnect() { return 0; }
	//// DTrader

	//// DTimerHandler 警告：子类重载该函数后，需要手动调用基类的该方法。
	virtual void OnTimeout( unsigned int timer_id );
	//// DTimerHandler

protected:
	//// DHttpHandler  警告：子类重载该函数后，需要手动调用基类的该方法。
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