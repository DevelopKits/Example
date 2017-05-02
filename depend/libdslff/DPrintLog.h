#ifndef	INCLUDED_LIBDSL_DPRINTLOG_H
#define	INCLUDED_LIBDSL_DPRINTLOG_H

#include <libdsl/dslbase.h>
#include <libdsl/DTime.h>
#include <libdsl/DMutex.h>
#include <stdarg.h>

// <1> �û�ʹ�õĺ�

// ��ӡ��־Ҫ��࣬ȥ��������Ϣ���ؼ����������ڲ��ң����ṩһЩԭʼ���ݹ�����

// ERR �� �������⣬��Ӧ�ó��ֵģ���Ҫ����ע�⣬��Ҫ�����
// INFO �� �����������ʼʱ���Զ�Щ�����ȶ�������𲽼��٣����ֻ�����ؼ��㣬����session���������٣�
// DEBUG �� ������Ϣ���Լ���������ķǹؼ�·��������Ƶ�����ֵĵط�����������²���ӡ��������ʹ�ã�����ҲҪ����

// ��־�ȼ��趨���ȼ�Խ�ߣ��������Խ��
#define DLOG_LEVEL_DEBUG	2
#define DLOG_LEVEL_INFO		4
#define DLOG_LEVEL_ERR		6

// ��־����
#define DLOG_SET_FILE( filename )	( dsl::DPrintLog::instance()->SetFile( filename ) )
#define DLOG_SET_STDERR( enable )	( dsl::DPrintLog::instance()->SetStderr( enable ) )
#define DLOG_SET_SYSLOG( enable )	( dsl::DPrintLog::instance()->SetSyslog( enable ) )
#define DLOG_SET_LEVEL( level ) ( dsl::DPrintLog::instance()->SetLevel( level ) )
// ���Ե�������ĳ���ض�ģ���level�����֧��64����ͨ��������Ҫ��ĳЩ�ض�ģ�������ϸ��־����
#define DLOG_SET_MODULE_LEVEL( module_name, level ) ( dsl::DPrintLog::instance()->SetModuleLevel( module_name, level ) )

// ����ģ����Զ����Լ���ģ�����ƣ���Ҫʱ�����ض����ˣ���CPP�ļ����ȶ���DLOG_MODULE�꣬�� #include <libdsl/DPrintLog.h>
#ifndef DLOG_MODULE
#define DLOG_MODULE ""
#endif

#define DLOG_MODULE_MAX_NAME_LEN	32
#define DLOG_MODULE_MAX_NUM			64

// ���������־�ĺ�
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

// ʹ����һ��ʹ������ĺ�Ϳ����ˣ�����Ҫʹ�������ʵ��

// <2> DLog��־��ʵ��

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
	// �����ض�module��level��-1��ʾɾ����module������ʱ�������ã�module == NULL��ʾ�����������
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
	int m_year, m_month, m_day;  // ��־�ļ����ڣ����ڱ仯�����½��ļ�
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
