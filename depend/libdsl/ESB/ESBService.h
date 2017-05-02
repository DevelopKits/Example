/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* ESBService.h : �������ߴ���ģ��
* ����    ��������
* ������ڣ�2014��9��29��
*
* ��ǰ�汾��1.0
*/

#pragma once

#include <libdsl/DHttpServer.h>
#include <libdsl/ESB/DMsgHandler.h>
#include <libdsl/ESB/DAutoPtr.h>
#include <libdsl/ESB/ISession.h>
#include <libdsl/ESB/DTrader.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class LIBDSL_API ESBService : public DMsgHandler, public DHttpHandler
{
public:
	ESBService();
	virtual ~ESBService();

	void SetTimeout(unsigned int nTime); // ��λΪ�룬Ĭ��30���ް���ʱ��

private:
	//// DMsgHandler
	virtual int OnInitHandler(); 
	virtual int OnAntinitHandler();
	virtual int OnBusRunning();
	virtual void OnTimer( unsigned int nTimerId );
	//// DMsgHandler

	//// DHttpHandler
	virtual int OnConnect( const DRef<DHttpSession> & httpsess, const char * remote_ip, int remote_port );
	virtual int OnHttp( const DRef<DHttpSession> & httpsess, const DHttp & httpmsg  );
	virtual int OnClose( const DRef<DHttpSession> & httpsess );
	//// DHttpHandler

	DECLARE_FUNC_MAP(ESBService)

private:
	DAutoPtr<DHttpServer>					m_pHttpServer;
	ISession<int, DRef<DTrader> >			m_senTradersInt;
	unsigned int							m_nTime;
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL