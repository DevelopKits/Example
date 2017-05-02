#ifndef	INCLUDED_LIBDSL_DPRINTLOG_H
#define	INCLUDED_LIBDSL_DPRINTLOG_H

#include <libdsl/dslbase.h>
#include <libdsl/DTime.h>
#include <libdsl/DMutex.h>
#include <stdarg.h>

// <1> 用户使用的宏

// 打印日志要简洁，去除冗余信息，关键字清晰便于查找，多提供一些原始数据供分析

// ERR ： 严重问题，不应该出现的，需要引起注意，需要处理的
// INFO ： 正常情况，开始时可以多些，随稳定性提高逐步减少，最后只保留关键点，例如session创建和销毁，
// DEBUG ： 调试信息，以及正常情况的非关键路径，或者频繁出现的地方，正常情况下不打印，供调试使用，数量也要控制

// 日志等级设定，等级越高，输出内容越少
#define DLOG_LEVEL_DEBUG	2
#define DLOG_LEVEL_INFO		4
#define DLOG_LEVEL_ERR		6

// 日志设置
#define DLOG_SET_FILE( filename )	( dsl::DPrintLog::instance()->SetFile( filename ) )
#define DLOG_SET_STDERR( enable )	( dsl::DPrintLog::instance()->SetStderr( enable ) )
#define DLOG_SET_SYSLOG( enable )	( dsl::DPrintLog::instance()->SetSyslog( enable ) )
#define DLOG_SET_LEVEL( level ) ( dsl::DPrintLog::instance()->SetLevel( level ) )
// 可以单独设置某个特定模块的level，最大支持64个，通常用于需要对某些特定模块进行详细日志分析
#define DLOG_SET_MODULE_LEVEL( module_name, level ) ( dsl::DPrintLog::instance()->SetModuleLevel( module_name, level ) )

// 具体模块可以定义自己的模块名称，需要时可以特定过滤，（CPP文件中先定义DLOG_MODULE宏，再 #include <libdsl/DPrintLog.h>
#ifndef DLOG_MODULE
#define DLOG_MODULE ""
#endif

#define DLOG_MODULE_MAX_NAME_LEN	32
#define DLOG_MODULE_MAX_NUM			64

// 具体输出日志的宏
#if defined(_MSC_VER) && _MSC_VER <= 1200
// VC6 doesn't support VARARG in macro, use some hack
#define DLOG_DEBUG	dsl::DPrintLogVC6( __FILE__, __LINE__, DLOG_MODULE, DLOG_LEVEL_DEBUG )
#define DLOG_INFO	dsl::DPrintLogVC6( __FILE__, __LINE__, DLOG_MODULE, DLOG_LEVEL_INFO )
#define DLOG_ERR	dsl::DPrintLogVC6( __FILE__, __LINE__, DLOG_MODULE, DLOG_LEVEL_ERR )
#else
// Normal
#define DLOG_ERR( fmt, ... )	( dsl::DPrintLog::instance()->Log( __FILE__, __LINE__, DLOG_MODULE, DLOG_LEVEL_ERR, fmt, ##__VA_ARGS__ ) )
#define DLOG_INFO( fmt, ... )	( dsl::DPrintLog::instance()->Log( __FILE__, __LINE__, DLOG_MODULE, DLOG_LEVEL_INFO, fmt, ##__VA_ARGS__ ) )
#define DLOG_DEBUG( fmt, ... )	( dsl::DPrintLog::instance()->Log( __FILE__, __LINE__, DLOG_MODULE, DLOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__ ) )
#endif

// 使用者一般使用上面的宏就可以了，不需要使用下面的实现

// <2> DLog日志库实现

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

#define DLOG_MAX_FNAME_LEN	(1000)
#define DLOG_MAX_LINE_LEN	(4096)

class DPrintLog
{
public:
	DPrintLog();
	~DPrintLog();

	static DPrintLog * instance();

	int SetFile( const char* szFileName );
	int SetStderr( bool enable );
	int SetSyslog( bool enable );
	int SetLevel( int level );
	// 设置特定module的level，-1表示删除，module数量满时不能设置，module == NULL表示清空所有设置
	int SetModuleLevel( const char * module, int level );

	int Log( const char * file, int line, const char * module, int level, const char * fmt, ... );
	int LogV( const char * file, int line, const char * module, int level, const char * fmt, va_list ap );

protected:
	int create_log_file( int year, int month, int day );
	bool MakeSureDirectoryExist(const char* dir);

private:
	static DPrintLog m_logger;

	bool m_enable_stderr;
	bool m_enable_syslog;

	DMutex m_mtxLock;
	int m_year, m_month, m_day;  // 日志文件日期，日期变化则重新建文件
	char m_fname[DLOG_MAX_FNAME_LEN];
#ifdef WIN32
	HANDLE m_fd;
#else
	int m_fd;
#endif

	int m_level;
	struct {
		char m_module[DLOG_MODULE_MAX_NAME_LEN];
		int m_level;
	} m_module_level[DLOG_MODULE_MAX_NUM];
};

class DPrintLogVC6
{
public:
	DPrintLogVC6( const char * log_file, int log_line, const char * module, int level ) 
		: m_log_file( log_file ), m_log_line( log_line ), m_module( module ), m_level( level ) { }
	~DPrintLogVC6() { }

	void operator () ( const char * fmt, ... );

private:
	const char * m_log_file;
	int m_log_line;
	const char * m_module;
	int m_level;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_DPRINTLOG_H
