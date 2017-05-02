/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2012-12
 *
 * DHttpServer.h : Http服务端库
 */

#ifndef INCLUDED_LIBDSL_DHTTPSERVER_H
#define INCLUDED_LIBDSL_DHTTPSERVER_H

#include <libdsl/DHttpClient.h>
#include <libdsl/DStr.h>

#include <vector>

class DStr;

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////



class LIBDSL_API DHttpServerSession : public DSocketHandler
{
public:
	DHttpServerSession( DNetEngine * ne, const DRef< DHttpHandler > & handler );

	int Start( const char * ip, int port );
	int Stop();

	int GetServerInfo( DStr * ip, int * port ) const;
	int SetHandler( const DRef< DHttpHandler > & handler );

protected:
	virtual void OnConnect( SOCKID hSocket, SOCKID newClient, const char * remoteAddr, int remotePort,DRef<DSocketHandler> * ppEvent );
	virtual void OnData( SOCKID hSocket, const char * pBuffer, int len, const char* remoteAddr, int remotePort );
	virtual void OnClose( SOCKID hSocket, int error );

	virtual ~DHttpServerSession();

protected:
	DNetEngine * m_neteng;
	DRef< DHttpHandler > m_handler;
	DStr m_ip;
	int m_port;
	SOCKID m_sock;
};

class LIBDSL_API DHttpServer
{
public:
	DHttpServer( DNetEngine * ne );
	virtual ~DHttpServer();

	int AddServer( const char * ip, int port, const DRef< DHttpHandler > & handler );
	int DelServer( const char * ip, int port );
	int DelAllServer();

	// 注意，DelServer()会影响idx顺序
	int GetServerNum();
	int GetServerInfo( int idx, DStr * ip, int * port );
	int SetServerHandler( int idx, const DRef< DHttpHandler > & handler );

protected:
	DNetEngine * m_neteng;
	DMutex m_lock;
	std::vector< DRef<DHttpServerSession> > m_vec_svr;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DHTTPSERVER_H
