#ifndef INCLUDED_LIBDSL_DTIME_H
#define INCLUDED_LIBDSL_DTIME_H

#include <libdsl/dslbase.h>
#include <time.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// 时间获取函数

class DTime
{
public:
	DTime();
	static int64_t GetTime();  // 静态成员函数 获取系统当前时间 单位ms Epoch(1970-1-1) 
	static uint32_t GetTick(); // ms from system start, 49 days roundup
	static int GetTimeZone();	// 获取系统时间与格林威治时间相差多少ms 单位ms
	
	void SetNow();
	void SetTime( int64_t t ); // 单位是ms，秒数要乘1000LL，要类型转换成64位，防止溢出
	int64_t MakeTime();		// 类成员的时间 单位ms Epoch(1970-1-1) 注意与GetTime()的区别

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

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_DTIME_H
