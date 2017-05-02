#ifndef INCLUDED_LIBDSL_DSYSUTIL_H
#define INCLUDED_LIBDSL_DSYSUTIL_H

#include <libdsl/dslbase.h>
#include <time.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class DSysUtil
{
public:  
	DSysUtil(){};
	~DSysUtil(){};

	static DSysUtil * instance(){return &m_sysUtil;}

	// 获取当前程序路径
	int getCurrentPath( char *path, int len );

	// 设置系统时间
	void syncTime( time_t newTime );

private:
	static DSysUtil m_sysUtil;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DSYSUTIL_H

