#ifndef INCLUDED_LIBDSL_DTIME_H
#define INCLUDED_LIBDSL_DTIME_H

#include <libdsl/dslbase.h>
#include <time.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// ʱ���ȡ����

class DTime
{
public:
	DTime();
	static int64_t GetTime();  // ��̬��Ա���� ��ȡϵͳ��ǰʱ�� ��λms Epoch(1970-1-1) 
	static uint32_t GetTick(); // ms from system start, 49 days roundup
	static int GetTimeZone();	// ��ȡϵͳʱ�����������ʱ��������ms ��λms
	
	void SetNow();
	void SetTime( int64_t t ); // ��λ��ms������Ҫ��1000LL��Ҫ����ת����64λ����ֹ���
	int64_t MakeTime();		// ���Ա��ʱ�� ��λms Epoch(1970-1-1) ע����GetTime()������

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
