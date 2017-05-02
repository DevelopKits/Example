/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2011-09
 *
 * DTime.h : 时间处理函数
 */

#ifndef INCLUDED_LIBDSL_DTIME_H
#define INCLUDED_LIBDSL_DTIME_H

#include <libdsl/dslbase.h>
#include <time.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

/* 注意：请小心 Y2038 问题，DTime支持转换的时间有一个范围，
 * linux 32位下大概是 1900 年到 2038 年，
 * linux 64位基本相当于无限制
 * windows平台（无论32位还是64位）大概是 1970 年到 3000 年
 */

#ifdef _WIN32
// windows平台（32位和64位）的 time_t 是64位的，但限制了一个时间范围（年份3000以内），再考虑时区
#define LIBDSL_DTIME_MAX ( (0x793406fffLL - 12LL * 3600) * 1000LL )
#define LIBDSL_DTIME_MIN ( 12LL * 3600 * 1000LL )
#elif ( defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 8 ) || ( defined(__WORDSIZE) && __WORDSIZE == 64 )
#define LIBDSL_DTIME_MAX ( 0x7fffffffffffffffLL )
#define LIBDSL_DTIME_MIN ( 0x8000000000000000LL )
#else
#define LIBDSL_DTIME_MAX ( 0x7fffffffLL * 1000 )
#define LIBDSL_DTIME_MIN ( 0xffffffff80000000LL * 1000 )
#endif

// DTime：时间的表示和转换的类

class LIBDSL_API DTime
{
public:
	// DTime类所支持（转换）的最大和最小时间
	static const int64_t s_maxTime = LIBDSL_DTIME_MAX;
	static const int64_t s_minTime = LIBDSL_DTIME_MIN;

	static int64_t Now();      // 静态成员函数 获取系统当前时间 单位ms Epoch(1970-1-1) 
	static int64_t GetTime() { return Now(); }
	static uint32_t GetTick(); // ms from system start, 49 days roundup
	static int GetTimeZone();	// 获取系统时间与格林威治时间相差多少ms 单位ms
	
	DTime();
	DTime( int64_t t );
	DTime( const DTime & t );
	DTime & operator = ( const DTime & t );

	void SetNow();

	// 单位是ms，秒数要乘1000LL，要类型转换成64位，防止溢出
	// 如果 t 小于LIBDSL_DTIME_MIN，则强制设为LIBDSL_DTIME_MIN所表示的时间。
	// 如果 t 大于LIBDSL_DTIME_MAX，则强制设置为LIBDSL_DTIME_MAX所表示的时间
	void SetTime( int64_t t ); 

	// 将类成员存放的本地时间转换成Epoch时间（从1970-1-1开始的秒数）单位ms
	// 如果实际表示的时间早于LIBDSL_DTIME_MIN所表示的时间，则返回LIBDSL_DTIME_MIN
	// 如果实际表示的时间晚于LIBDSL_DTIME_MAX所表示的时间，则返回LIBDSL_DTIME_MAX
	int64_t MakeTime() const;

	// t 是时间间隔，单位ms
	DTime & operator += ( int64_t t );
	DTime & operator -= ( int64_t t );

public:
	int m_year;		
	int m_month;	
	int m_day;		
	int m_hour;		
	int m_minute;
	int m_second;
	int m_millisecond;
	int m_dayOfWeek;
};

LIBDSL_API bool operator == ( const DTime & t1, const DTime & t2 );
LIBDSL_API bool operator != ( const DTime & t1, const DTime & t2 );
LIBDSL_API bool operator < ( const DTime & t1, const DTime & t2 );
LIBDSL_API bool operator <= ( const DTime & t1, const DTime & t2 );
LIBDSL_API bool operator > ( const DTime & t1, const DTime & t2 );
LIBDSL_API bool operator >= ( const DTime & t1, const DTime & t2 );
LIBDSL_API DTime operator + ( const DTime & t1, int64_t t2 );
LIBDSL_API DTime operator - ( const DTime & t1, int64_t t2 );

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_DTIME_H
