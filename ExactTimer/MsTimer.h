#ifndef __MS_TIMER
#define __MS_TIMER

/* C++标准库头文件 */
#include <list>
#include <map>
#include <windows.h>

/* 定时器的定时精度范围 */
#define OSSYSTIME_MININTERVAL (1)
#define OSSYSTIME_MAXINTERVAL (65535)
#define IMOS_UNUSED_ARG(x)    ((VOID) x)

/* 定时器尝试最大次数 */
#define OSTOTAL_TIMERID_TRYNUM (1000)

/* 定时器Flag标记 */
#define  OSTIME_KILL_SYNCHRONOUS  0x1
#define  OSTIME_PERIODIC 0x2
#define  OSTIME_ADJUST_PERIODIC   0x4    //用户在回调函数中能够动态修改定时器下次触发的时间间隔

/* 定义定时器函数的错误码 */
#define  OSTIME_NO_ERR  0
#define  OSTIME_ERR     1

#define  OSTIME_PREC                 1000     //当前定时器的精度细分
#define  OSTIME_MAX_DELAY_CYC        10       //当定时器延时超过10个周期时，需要进行定时器时间复位
#define  OSTIME_33MS_ADJUST_TIME_VAL 40       //对周期为33ms的进行周期修改值，用于规避IPC 30帧每秒时帧率不足问题

/**
* 定时器回调函数类型定义
* @param [IN] ulTimerID 执行回调函数的定时器ID号
* @param [IN] ulmsg 参数未使用
* @param [IN] ulUser 定时器回调函数携带的参数信息，该参数由OstimeSetEvent函数的ulUser参数传入
* @param [IN] ul1 参数未使用
* @param [IN] ul2 参数未使用
* @return 无
*/
typedef VOID (* LPOSTIMECALLBACK)(IN unsigned int ulTimerID, IN unsigned int ulmsg, IN ULONG ulUser, IN unsigned int ul1, IN unsigned int ul2);

/* 定时器中的list队列中的控制块结构 */
typedef struct tagOS_TIMERENTRY
{
	unsigned int             ulDelay;  /* 等待的时间 */
	unsigned int             ulResol;
	LPOSTIMECALLBACK  lpFunc; /* 该定时器的回调函数 */
	ULONG             ulUser;
	unsigned int             ulFlags;
	unsigned int             ulTimerID;
	LONGLONG          llTriggerTime;
	LPVOID arg;
	struct tagOS_TIMERENTRY*  pstNextOsTimerEntry;
} STOSTIMERENTRY, *PSTOSTIMERENTRY;


class MsTimer
{
public:
	MsTimer(void);
	~MsTimer(void);
public:
	unsigned int OSTIME_TimeStart(VOID);
	VOID OSTIME_TimeStop(VOID);
	unsigned int OstimeSetEvent(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags);
	
private:
	static LONGLONG  OsGetTickCount64(LPVOID arg);
	static DWORD CALLBACK OSTIME_SysTimeThread(IN LPVOID arg);
	static unsigned int OSTIME_SysTimeCallback(IN LPVOID arg);
	static VOID OSTIME_TriggerCallBack(IN PSTOSTIMERENTRY lpTimer);
	unsigned int OSTIME_SetEventInternal(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags);
	unsigned int OsGenTimerID(VOID);
	VOID OsReleaseTimerID(IN unsigned int ulTimerID);
	unsigned int OstimeKillEvent(IN unsigned int ulTimerID);

private:
	/* 定时器变量 */
	HANDLE                 m_hOsTimeMMTimer ;   /*定时器线程句柄*/
	PSTOSTIMERENTRY        m_pstOsTimersList ;  /*定时器控制队列*/
	PSTOSTIMERENTRY        m_pstTimersArray ;   /*用于保存回调函数的数组*/
	int                    m_lSizeLpTimers;     /*定时器回调函数数组的内存长度*/
	HANDLE                 m_hOsTimeKillEvent ; /*定时器同步退出标记*/
    HANDLE                 m_hOsTimeWakeEvent ; /*定时器线程触发事件*/
	BOOL                   m_bOsTimeToDie;      /*定时器线程退出标记*/
	HANDLE                 m_hOSTimeHeap;       /*用于定时器模块内存分配堆*/
	/* 跟TimerID生成有关的变量 */
	std::map<unsigned int, unsigned int> m_mapOsTimerID;              /*定时器ID维护队列*/
    CRITICAL_SECTION       m_OSTimerID_cs;              /*定时器ID维护关键段*/
    CRITICAL_SECTION       m_GetSystemTime_cs;          /*定时器相对时间获取关键段*/
    CRITICAL_SECTION       m_OsTime_cs;                 /*定时器控制块维护关键段*/
    LARGE_INTEGER          m_lgCurTime;                 /*系统启动后的相对时间*/
    unsigned int           m_ulCurTimerID ;          /*定一个全局的定时器ID号 */

	unsigned long          m_ulTimerID; //定时器ID

};
#endif