/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2012-12
 *
 * DHttpClient.h : Http客户端库
 */

#ifndef INCLUDED_LIBDSL_DHTTPCLIENT_H
#define INCLUDED_LIBDSL_DHTTPCLIENT_H

#include <libdsl/dslbase.h>
#include <libdsl/DHttp.h>
#include <libdsl/DRefObj.h>
#include <libdsl/DEvent.h>
#include <libdsl/DNetEngine.h>

#include <deque>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

#define HTTP_SEND_MAX_SIZE			50 * 1024 * 1024	// 一次最大50MB
class DHttpSession;
class DHttpClient;
class DHttpServerSession;

class LIBDSL_API DHttpHandler : virtual public DRefObj
{
public:
	DHttpHandler() {}
	virtual ~DHttpHandler() {}

	virtual int OnConnect( const DRef<DHttpSession> & httpsess, const char * remote_ip, int remote_port ) = 0;
	virtual int OnHttp( const DRef<DHttpSession> & httpsess, const DHttp & httpmsg  ) = 0;
	virtual int OnClose( const DRef<DHttpSession> & httpsess ) = 0;
};

class LIBDSL_API DHttpSession : public DSocketHandler
{
public:
	DHttpSession();

	int SetHandler( const DRef< DHttpHandler > & handler );
	DRef<DHttpHandler> GetHandler();

	int Send( const char* szBuf, int nLen );
	int SendHttp( const DHttp & httpmsg );
	// return 1 : got msg, 0 : timeout, < 0 : error
	int WaitHttp( DHttp * httpmsg, int timeout_ms );
	int Close();

protected:

	virtual void OnConnect( SOCKID hSocket, SOCKID newClient, const char * remoteAddr, int remotePort,DRef<DSocketHandler> * ppEvent );
	virtual void OnData( SOCKID hSocket, const char * pBuffer, int len, const char* remoteAddr, int remotePort );
	virtual void OnClose( SOCKID hSocket, int error );

	virtual ~DHttpSession();
	
protected:
	friend class DHttpClient;
	friend class DHttpServerSession;
	DNetEngine * m_neteng;
	SOCKID m_sock;
	DRef<DHttpHandler> m_handler; // 如果有handler，则通过handler回调
	DHttp * m_msg;		// 缓存未收完整的http包
	bool m_is_recv_body;

	DEvent m_evt;
	DHttp * m_wait_msg;  // 如果外部正在调用WaitHttp在等待，则进行通知等待成功
	std::deque< DHttp*> m_pending_msg;  // 否则把消息缓存起来供后续查询
	DMutex m_mtxSend;
};

class LIBDSL_API DHttpClient
{
public:
	DHttpClient( DNetEngine * ne );
	virtual ~DHttpClient();

	DRef<DHttpSession> Connect( const char * ip, int port, const DRef<DHttpHandler> & handler = 0 );

protected:
	DNetEngine * m_neteng;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DHTTPCLIENT_H
