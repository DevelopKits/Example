/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2014-04
 *
 * DStatusServer.h : 状态服务
 */

#ifndef INCLUDED_LIBDSL_DSTATUSSERVER_H
#define INCLUDED_LIBDSL_DSTATUSSERVER_H

#include <libdsl/DHttpServer.h>

#include <set>
#include <list>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

#define STATUS_CONTENT_TYPE_DIR		"x-status/dir"
#define STATUS_CONTENT_TYPE_INFO1	"x-status/info1"

class LIBDSL_API DStatusServer : public DHttpHandler
{
public:
	DStatusServer( DNetEngine * ne );

	int SetDocRoot( const char * local_path ); // default is "./html"
	// TODO : auth

	// path应该是根目录下的文件或者目录，不支持多层目录
	int AddResHdl( const char * path, const DRef<DHttpHandler> & handler, const char * content_type = STATUS_CONTENT_TYPE_DIR );
	int DelResHdl( const char * path );

	int Start( const char * ip, int port );
	int Stop();

	static int ReplyHttp( const DRef<DHttpSession> & httpsess, const DHttp & httpmsg, int status, const char * content_type, const char * data, int len = -1 );

protected: // IMP DHttpHandler
	virtual int OnConnect( const DRef<DHttpSession> & httpsess, const char * remote_ip, int remote_port );
	virtual int OnHttp( const DRef<DHttpSession> & httpsess, const DHttp & httpmsg  );
	virtual int OnClose( const DRef<DHttpSession> & httpsess );

protected:
	virtual ~DStatusServer();

	DRef<DHttpHandler> find_handler( const DStr & path );

	// 返回 0：未处理；> 0：成功处理；< 0：处理失败
	int handle_status( const DStr & path, const DStr & param, const DRef<DHttpSession> & httpsess, const DHttp & httpmsg  );
	int handle_libdsl( const DStr & path, const DStr & param, const DRef<DHttpSession> & httpsess, const DHttp & httpmsg  );
	int handle_dir( const DStr & path, const DStr & param, const DRef<DHttpSession> & httpsess, const DHttp & httpmsg  );

protected:
	DHttpServer m_svr;

	DStr m_docroot;

	DMutex m_lock;
	class ResHdlInfo
	{
	public:
		DStr m_path;
		DRef<DHttpHandler> m_hdl;
		DStr m_type;
	};
	std::list< ResHdlInfo > m_hdls; // 数量不会太多，先简单处理，使用链表，按m_path从长到短排序
	std::set< DRef<DHttpSession> > m_sess;  // 当前在线的用户信息
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DSTATUSSERVER_H
