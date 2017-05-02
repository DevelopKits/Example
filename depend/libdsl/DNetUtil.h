/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2013-12
 *
 * DNetUtil.h : util function for network
 */

#ifndef	INCLUDED_LIBDSL_DNETUTIL_H
#define	INCLUDED_LIBDSL_DNETUTIL_H

#include <libdsl/DNetEngine.h>  // DSL_NET_ADDR_LEN

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class DStr;

struct RemoteAddrStat
{
	char	addr[DSL_NET_ADDR_LEN];
	int		port;
	bool	reachable;
};

#define DNETUTIL_CHECKTCP_WAITALL	(1<<0)

class LIBDSL_API DNetUtil
{
public:
	DNetUtil() {}
	~DNetUtil() {}

	// 域名解析，将host域名解析成IP地址（数字形式）
	static int ResolveAddr(const char * host, char retAddr[DSL_NET_ADDR_LEN]);

	// 并发检查TCP连通性，缺省是当有一成功、超时、所有失败时返回
	// 如果flag为DNETUTIL_CHECKTCP_WAITALL，则全部都成功或失败、超时才返回
	static int CheckTcpConnect(RemoteAddrStat * addressArray, int arraySize, int timeout_ms = 3000, int flag = 0 );

	// 解析URL，得到IP和Port
	// 支持格式: [schema://]host:[port]/XXX，支持IPv4 & IPv6，如果port字段不存在，那么使用defaultPort的值
	static int ParseIpPortFromUrl(const char * url, int defaultPort, DStr * retHost, int * retPort );
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DNETUTIL_H
