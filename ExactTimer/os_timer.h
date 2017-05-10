
/* 实现定时器的实现函数 */
#ifndef _XP_OS_TIMER_H_
#define _XP_OS_TIMER_H_

/* 定时器尝试最大次数 */
#define OSTOTAL_TIMERID_TRYNUM (1000)
#define IN 
#define OUT
#define INOUT

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
    struct tagOS_TIMERENTRY*  pstNextOsTimerEntry;
} STOSTIMERENTRY, *PSTOSTIMERENTRY;


/* 定时器库对外函数接口 */

/**
* 定时器内部线程运行启动函数,在启动定时前必须调用的函数。不支持多线程调用，必须同OSTIME_TimeStop配套使用
* @param [IN] 无
* @return OSTIME_NO_ERR 返回成功
*         OSTIME_ERR    返回失败
*/
unsigned int OSTIME_TimeStart(VOID);

/**
* 定时器内部线程关闭函数，在退出时必须调用的函数.不支持多线程调用，必须同OSTIME_TimeStart配套使用
* @param [IN] 无
* @return 无
*/
VOID OSTIME_TimeStop(VOID);

/**
* 启动一个定时器
* @param [IN] ulDelay 定时器的周期，单位ms，范围必须为[1、65535].
* @param [IN] ulResol 为兼容windows操作系统的函数，参数未启用。
* @param [IN] lpFunc 回调函数，必须为LPOSTIMECALLBACK的函数指针
* @param [IN] ulUser 传递给回调函数的参数。
* @param [IN] ulFlags 标识当前定时器的类型，现在只支持两种标记:
              OSTIME_KILL_SYNCHRONOUS  当前为同步定时器，OstimeKillEvent返回时必须保证不再进行回调函数调用
              OSTIME_PERIODIC 是否需要周期调用回调函数，不设置则属于一次性定时器
* @return 定时器ID号，如果失败，则返回的定时器ID号为0，成功返回非0值
*/
unsigned int OstimeSetEvent(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags);

/**
* 停止一个定时器
* @param [IN] ulTimerID 定时器ID号。
* @return OSTIME_ERR 表示删除定时器失败
*      OSTIME_NO_ERR 表示删除定时器成功
*/
unsigned int OstimeKillEvent(IN unsigned int ulTimerID);

#endif

