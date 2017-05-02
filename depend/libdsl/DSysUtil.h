/* Copyright (c) 2011, 浙江大华技术股份有限公司,  All rights reserved.
 *2014-03
 *
 * 文件名称：DSysUtil.h
 */

#ifndef INCLUDED_LIBDSL_DSYSUTIL_H
#define INCLUDED_LIBDSL_DSYSUTIL_H

#include <libdsl/dslbase.h>
#include <time.h>
#include <vector>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

struct ProcessInfo
{
	int Pid;
	char ProcName[64];
	char ProcPath[256];
};

class LIBDSL_API DSysUtil
{
public:  
	DSysUtil(){};
	~DSysUtil(){};

	static DSysUtil * instance(){return &m_sysUtil;}

	// 获取当前程序路径，获取成功返回0
	int getCurrentPath( char *path, int len );

	// 设置系统时间
	void syncTime( time_t newTime );

	// 通过进程名称获取进程信息,若名称为NULL,则获取当前所有进程的信息
	int GetProcInfo( std::vector<dsl::ProcessInfo> &vecProcInfo, char *name = NULL );

private:
	static DSysUtil m_sysUtil;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DSYSUTIL_H

