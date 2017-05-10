/* C++标准库头文件 */
#include <list>
#include <map>
#include <windows.h>

/* 定时器的头文件 */
#include "os_timer.h"


/* 定时器的定时精度范围 */
#define OSSYSTIME_MININTERVAL (1)
#define OSSYSTIME_MAXINTERVAL (65535)
#define IMOS_UNUSED_ARG(x)    ((VOID) x)

/* 定时器的全局变量 */
static    HANDLE                 gshOsTimeMMTimer = (HANDLE) NULL;     /*定时器线程句柄*/
static    PSTOSTIMERENTRY        gspstOsTimersList = NULL;             /*定时器控制队列*/
static    PSTOSTIMERENTRY        gspstTimersArray = NULL;              /*用于保存回调函数的数组*/
static    int                    gslSizeLpTimers = 0;                  /*定时器回调函数数组的内存长度*/
static    HANDLE                 gshOsTimeKillEvent = (HANDLE) NULL;   /*定时器同步退出标记*/
static    HANDLE                 gshOsTimeWakeEvent = (HANDLE) NULL;   /*定时器线程触发事件*/
static    BOOL                   gsbOsTimeToDie = TRUE;                /*定时器线程退出标记*/
static    HANDLE                 gshOSTimeHeap = (HANDLE) NULL;        /*用于定时器模块内存分配堆*/

/* 跟TimerID生成有关的全局变量 */
static    std::map<unsigned int, unsigned int> gsmapOsTimerID;              /*定时器ID维护队列*/
static    CRITICAL_SECTION       gsOSTimerID_cs;              /*定时器ID维护关键段*/
static    CRITICAL_SECTION       gsGetSystemTime_cs;          /*定时器相对时间获取关键段*/
static    CRITICAL_SECTION       gsOsTime_cs;                 /*定时器控制块维护关键段*/
static    LARGE_INTEGER          gslgCurTime;                 /*系统启动后的相对时间*/
static    unsigned int           gsulCurTimerID = 0;          /*定一个全局的定时器ID号 */


/**
* 包装windows定时器支持获得64位时间信息，注:该函数需要外部保证不进行多线程调用
* @param [IN] 无
* @return 当前的系统时间，返回的为相对时间，精度为ms，
*/
LONGLONG  OsGetTickCount64(VOID)
{
	DWORD dwCurTime = timeGetTime();

	/* 判断时间是否产生溢出 */
	if (dwCurTime < gslgCurTime.LowPart)
	{
		gslgCurTime.HighPart++;
	}
	gslgCurTime.LowPart = dwCurTime;
	return  gslgCurTime.QuadPart*OSTIME_PREC;
}


/**
* 线程启动回调函数，该函数有定时器线程调用
* @param [IN] arg 没有使用
* @return 返回值为window线程启动函数固有。
*/
static DWORD CALLBACK OSTIME_SysTimeThread(IN LPVOID arg);

/**
* 定时器线程的执行函数，被上面线程回调函数调用
* @param [IN] 无
* @return 当前定时器链表需要等待的间隔时间,单位为ms
*/
static unsigned int OSTIME_SysTimeCallback(VOID);

/**
* 用户注册给定时器的定时器回调函数
* @param [IN] lpTimer 定时器控制块的地址
* @return 无
*/
static VOID OSTIME_TriggerCallBack(IN PSTOSTIMERENTRY lpTimer);

/**
* 启动一个定时器，定时器模块内部实现函数
* @param [IN] ulDelay 定时器的周期，单位ms，范围必须为澹[1、65535].
* @param [IN] ulResol 为兼容windows操作系统的函数，参数未启用。
* @param [IN] lpFunc 回调函数，必须为LPOSTIMECALLBACK的函数指针
* @param [IN] ulUser 传递给回调函数的参数。
* @param [IN] ulFlags 标识当前定时器的类型，现在只支持两种标记:
              OSTIME_KILL_SYNCHRONOUS  当前为同步定时器，OstimeKillEvent返回时必须保证不再进行回调函数调用
              OSTIME_PERIODIC 是否需要周期调用回调函数，不设置则属于一次性定时器
* @return 定时器ID号，如果失败，则返回的定时器ID号为0，成功返回非0值
*/
unsigned int OSTIME_SetEventInternal(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags);


/**
* 使用C++的map产生一个定时器ID号
* @param [IN] 无
* @return 返回当前的定时器ID。0 表示获取定时器ID失败
*/
unsigned int OsGenTimerID(VOID)
{
    /*查询队列中是否已经含有定时器ID号,定时器ID不能为0,同时尝试查询的次数为OSTOTAL_TIMERID_TRYNUM次*/
    std::map<unsigned int, unsigned int>::iterator iter;

    EnterCriticalSection(&gsOSTimerID_cs);
    gsulCurTimerID++;
    if (0 == gsulCurTimerID)
    {
        gsulCurTimerID++;
    }

    for (unsigned int ulIndex = 0; ulIndex < OSTOTAL_TIMERID_TRYNUM; ulIndex++)
    {
        iter = gsmapOsTimerID.find(gsulCurTimerID);
        if (gsmapOsTimerID.end() != iter)
        {
            gsulCurTimerID++;
            if (0 == gsulCurTimerID)
            {
                gsulCurTimerID++;
            }
        }
        else
        {
            /* 将获得的定时器Timer压入map队列 */
            gsmapOsTimerID.insert(std::pair<unsigned int, unsigned int>(gsulCurTimerID, gsulCurTimerID));

            unsigned int ulTimerIDTmp = gsulCurTimerID;
            LeaveCriticalSection(&gsOSTimerID_cs);
            return ulTimerIDTmp;
        }
    }
    LeaveCriticalSection(&gsOSTimerID_cs);
    return 0;
}

/**
* 删除全局队列中的timerID编号
* @param [IN] ulTimerID需要删除的定时器ID号
* @return 无
*/
VOID OsReleaseTimerID(IN unsigned int ulTimerID)
{
    EnterCriticalSection(&gsOSTimerID_cs);
    gsmapOsTimerID.erase(ulTimerID);
    LeaveCriticalSection(&gsOSTimerID_cs);
    return;
}

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
unsigned int OstimeSetEvent(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags)
{
    unsigned int ulFps = (1000/ulDelay);
    unsigned int msInterval = (1000 * OSTIME_PREC)/ulFps;

    /* 对33ms为周期的定时器进行微调,规避IPC 30帧每秒时帧率不足问题 */
    if (ulDelay == 33)
    {
        msInterval += OSTIME_33MS_ADJUST_TIME_VAL;
    }

    return OSTIME_SetEventInternal(msInterval, ulResol, lpFunc, ulUser, ulFlags);
}

/**
* 停止一个定时器
* @param [IN] ulTimerID 定时器ID号。
* @return OSTIME_ERR 表示删除定时器失败
*      OSTIME_NO_ERR 表示删除定时器成功
*/
unsigned int OstimeKillEvent(IN unsigned int ulTimerID)
{
    PSTOSTIMERENTRY  pstSelfTimerEntry = NULL;
    PSTOSTIMERENTRY  *ppstTimerEntry = NULL;

    EnterCriticalSection(&gsOsTime_cs);

    /* 遍历列表寻找定时器对象 */
    for (ppstTimerEntry = &gspstOsTimersList; *ppstTimerEntry; ppstTimerEntry = &(*ppstTimerEntry)->pstNextOsTimerEntry)
    {
        if (ulTimerID == (*ppstTimerEntry)->ulTimerID)
        {
            pstSelfTimerEntry = *ppstTimerEntry;
            *ppstTimerEntry = (*ppstTimerEntry)->pstNextOsTimerEntry;
            break;
        }
    }
    LeaveCriticalSection(&gsOsTime_cs);

    if (NULL == pstSelfTimerEntry)
    {
        /* 表示关闭定时器失败 */
 /*       dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OstimeKillEvent: Cann't find OsTimer[TimerID: %u] in gspstOsTimersList\n",
                ulTimerID);*/
        return OSTIME_ERR;
    }

    /* 同步退出需要等待回调函数调用完成 */
    if (0 != (pstSelfTimerEntry->ulFlags & OSTIME_KILL_SYNCHRONOUS))
    {
        DWORD dwRetVal = WaitForSingleObject(gshOsTimeKillEvent, INFINITE);
        if ((WAIT_TIMEOUT != dwRetVal) && (WAIT_OBJECT_0 != dwRetVal))
        {
            /* 定时器线程运行出现了异常，现在需要进行退出 */
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OstimeKillEvent: The pstTimerEntry ID %u will exit with Calling WaitForSigleObject function! ErrorCode:%u\n",
                    ulTimerID, GetLastError());*/
        }
    }

    /* 需要归还分配的定时器ID号 */
    OsReleaseTimerID(pstSelfTimerEntry->ulTimerID);
    HeapFree(gshOSTimeHeap, 0, pstSelfTimerEntry);
    pstSelfTimerEntry = NULL;

    return OSTIME_NO_ERR;
}


/**
* 启动一个定时器，定时器模块内部实现函数
* @param [IN] ulDelay 定时器的周期，单位ms，范围必须为[1、65535].
* @param [IN] ulResol 为兼容windows操作系统的函数，参数未启用。
* @param [IN] lpFunc 回调函数，必须为LPOSTIMECALLBACK的函数指针
* @param [IN] ulUser 传递给回调函数的参数。
* @param [IN] ulFlags 标识当前定时器的类型，现在只支持两种标记:
              OSTIME_KILL_SYNCHRONOUS  当前为同步定时器，OstimeKillEvent返回时必须保证不再进行回调函数调用
              OSTIME_PERIODIC 是否需要周期调用回调函数，不设置则属于一次性定时器
* @return 定时器ID号，如果失败，则返回的定时器ID号为0，成功返回非0值
*/
unsigned int OSTIME_SetEventInternal(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags)
{
    /* 判断定时器周期的有效性 */
    if ((ulDelay < OSSYSTIME_MININTERVAL) || (ulDelay > OSSYSTIME_MAXINTERVAL) || (NULL == lpFunc))
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OSTIME_SetEventInternal: The ulDelay paremeter in OsTimer is invalid\n");*/
        return 0;
    }

    /* 申请一块定时器控制块 */
    PSTOSTIMERENTRY pstNewTimer = (PSTOSTIMERENTRY) HeapAlloc(gshOSTimeHeap, 0, sizeof(STOSTIMERENTRY));
    if (NULL == pstNewTimer)
    {
      /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OSTIME_SetEventInternal: Cann't HeapAlloc a memory for pstTimerEntry contrl\n");*/
        return 0;
    }

    pstNewTimer->ulDelay = ulDelay;

    EnterCriticalSection(&gsGetSystemTime_cs);
    pstNewTimer->llTriggerTime = OsGetTickCount64() + ulDelay;
    LeaveCriticalSection(&gsGetSystemTime_cs);

    pstNewTimer->ulResol = ulResol;
    pstNewTimer->lpFunc = lpFunc;
    pstNewTimer->ulUser = ulUser;
    pstNewTimer->ulFlags = ulFlags;

    EnterCriticalSection(&gsOsTime_cs);
    /* 判断是否创建同步定时器，如果时同步定时器并且停止事件没有创建则进行事件创建 */
    if ((0 != (ulFlags & OSTIME_KILL_SYNCHRONOUS)) && ((HANDLE) NULL == gshOsTimeKillEvent))
    {
        gshOsTimeKillEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
        if ((HANDLE) NULL == gshOsTimeKillEvent)
        {
           /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SetEventInternal: Failed to create an Time kill Event for pstTimerEntry running! ErrorCode:%u\n",
                    GetLastError());*/
            LeaveCriticalSection(&gsOsTime_cs);
            HeapFree(gshOSTimeHeap, 0, pstNewTimer);
            pstNewTimer = NULL;

            return 0;
        }
    }

    pstNewTimer->ulTimerID = OsGenTimerID();
    if (0 == pstNewTimer->ulTimerID)
    {
        LeaveCriticalSection(&gsOsTime_cs);
        HeapFree(gshOSTimeHeap, 0, pstNewTimer);
        pstNewTimer = NULL;
     /*   dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OSTIME_SetEventInternal: Cann't general an timerID for pstTimerEntry contrl\n");*/
        return 0;
    }
    pstNewTimer->pstNextOsTimerEntry = gspstOsTimersList;
    gspstOsTimersList = pstNewTimer;
    LeaveCriticalSection(&gsOsTime_cs);

    /* 触发一次定时回调函数调用 */
    if (0 == SetEvent(gshOsTimeWakeEvent))
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_WARNING,
                "[WARNING] OSTIME_SetEventInternal: Failed to set gshOsTimeWakeEvent event to the timer running thread! ErrorCode:%u\n",
                GetLastError());*/
    }

    return pstNewTimer->ulTimerID;
}

/**
* 用户注册给定时器的定时器回调函数
* @param [IN] lpTimer 定时器控制块的地址
* @return 无
*/
static VOID OSTIME_TriggerCallBack(IN PSTOSTIMERENTRY lpTimer)
{
    /* 根据用户设定的定时器flag标记位确认是否将结构体传递给用户 */
    if (lpTimer->ulFlags & OSTIME_ADJUST_PERIODIC)
    {
        STOSTIMERENTRY stOsTimerEntry = *lpTimer;
        (lpTimer->lpFunc)(lpTimer->ulTimerID, 0, lpTimer->ulUser, 0, (unsigned int) lpTimer);

        /* 用户修改了定时器周期后，需要更新定时器控制块 */
        if (stOsTimerEntry.ulDelay != lpTimer->ulDelay)
        {
            EnterCriticalSection(&gsGetSystemTime_cs);
            for (PSTOSTIMERENTRY pStOsTimerEntry = gspstOsTimersList; pStOsTimerEntry != NULL; pStOsTimerEntry = pStOsTimerEntry->pstNextOsTimerEntry)
            {
                if (pStOsTimerEntry->ulTimerID == lpTimer->ulTimerID)
                {
                   /* dsp_log(MOD_OSTIME, DSP_LOG_TRACE,
                            "[INIFO] %d OSTIME_SysTimeCallback: change from %d to %d\n",
                            pStOsTimerEntry->ulTimerID, pStOsTimerEntry->ulDelay, lpTimer->ulDelay);*/
                    pStOsTimerEntry->ulDelay = lpTimer->ulDelay;
                }
            }
            LeaveCriticalSection(&gsGetSystemTime_cs);
        }
    }
    else
    {
        (lpTimer->lpFunc)(lpTimer->ulTimerID, 0, lpTimer->ulUser, 0, 0);
    }
}

/**
* 定时器线程的执行函数，被上面线程回调函数调用
* @param [IN] 无
* @return 当前定时器链表需要等待的间隔时间,单位为ms
*/
static unsigned int OSTIME_SysTimeCallback(VOID)
{
    PSTOSTIMERENTRY     pstTimerEntry = NULL;
    PSTOSTIMERENTRY     *ppstTimerEntry = NULL;
    PSTOSTIMERENTRY     *ppstNextTimerEntry = NULL;
    int                lIndex = 0;
    LONGLONG            llCurTime = 0;
    DWORD               dwDeltaTime = 0;
    DWORD               dwRetTime = INFINITE;
    DWORD               dwAdjustTime = 0;
    LARGE_INTEGER       lgTempTime;
    lgTempTime.QuadPart = 0;

    /* 定时器队列为空，现在直接退出 */
    if (NULL == gspstOsTimersList)
    {
        return(dwRetTime);
    }

    /* 获得当前的系统时间，进行时间校正使用 */
    EnterCriticalSection(&gsGetSystemTime_cs);
    llCurTime = OsGetTickCount64();
    LeaveCriticalSection(&gsGetSystemTime_cs);

    /* 扫描整个列表，计算新的时间戳信息，将需要回调的对象放置到一个数组中进行回调 */
    EnterCriticalSection(&gsOsTime_cs);
    for (ppstTimerEntry = &gspstOsTimersList; *ppstTimerEntry != NULL; )
    {
        pstTimerEntry = *ppstTimerEntry;
        ppstNextTimerEntry = &pstTimerEntry->pstNextOsTimerEntry;
        if (llCurTime >= pstTimerEntry->llTriggerTime)
        {
            if (NULL != pstTimerEntry->lpFunc)
            {
                if (lIndex == gslSizeLpTimers)
                {
                    /* 标识当前的定时器存放空间已经不足，需要进行动态申请 */
                    if (NULL != gspstTimersArray)
                    {
                        gspstTimersArray = (PSTOSTIMERENTRY) HeapReAlloc(gshOSTimeHeap, 0, gspstTimersArray, ((DWORD)(++gslSizeLpTimers)) * sizeof(STOSTIMERENTRY));
                        if (NULL == gspstTimersArray)
                        {/*
                            dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                                    "[ERROR] OSTIME_SysTimeCallback: Failed to call HeapReAlloc with %u Bytes\n",
                                    (DWORD) gslSizeLpTimers * sizeof(STOSTIMERENTRY));*/
                            lIndex = 0;
                            gslSizeLpTimers = 0;
                            break;
                        }
                    }
                    else
                    {
                        gspstTimersArray = (PSTOSTIMERENTRY) HeapAlloc(gshOSTimeHeap, 0, ((DWORD)(++gslSizeLpTimers)) * sizeof(STOSTIMERENTRY));
                        if (NULL == gspstTimersArray)
                        {
                         /*   dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                                    "[ERROR] OSTIME_SysTimeCallback: Failed to call HeapAlloc with %d Bytes\n",
                                    (DWORD) gslSizeLpTimers * sizeof(STOSTIMERENTRY));*/
                            lIndex = 0;
                            gslSizeLpTimers = 0;
                            break;
                        }
                    }
                }
                gspstTimersArray[lIndex++] = *pstTimerEntry;

            }

            /* 更新当前触发时间为下次定时器到的时间  */
            pstTimerEntry->llTriggerTime += pstTimerEntry->ulDelay;

            /* 定时器设置为单次定时器 */
            if (0 == (pstTimerEntry->ulFlags & OSTIME_PERIODIC))
            {
                /* 从定时器链表中删除当前的定时器 */
                *ppstTimerEntry = *ppstNextTimerEntry;
                HeapFree(gshOSTimeHeap, 0, pstTimerEntry);
                pstTimerEntry = NULL;

                /* 将当前定时器的等待时间设置为无穷 */
                dwDeltaTime = INFINITE;
            }
            else
            {
                /* 计算下次中的时间间隔 */
                if (pstTimerEntry->llTriggerTime <= llCurTime)
                {
                    /* 当定时器需要追赶的时间差超过设置的时间间隔时，需要将定时器时间戳同步为当前系统时间 */
                    if ((unsigned int) (llCurTime - pstTimerEntry->llTriggerTime) > (OSTIME_MAX_DELAY_CYC * pstTimerEntry->ulDelay))
                    {
                       /* dsp_log(MOD_OSTIME, DSP_LOG_INFO,
                                "[ERROR] OSTIME_SysTimeCallback: Reset Trigger time to current system time for diff time[%u]\n",
                                llCurTime - pstTimerEntry->llTriggerTime);*/
                        pstTimerEntry->llTriggerTime = llCurTime;
                    }

                    dwDeltaTime = 0;
                }
                else
                {
                    lgTempTime.QuadPart = pstTimerEntry->llTriggerTime - llCurTime;
                    dwDeltaTime = lgTempTime.LowPart;
                }
            }
        }
        else
        {
            /* 如果当前的定时器没有命中，直接修改定时周期即可 */
            lgTempTime.QuadPart = pstTimerEntry->llTriggerTime - llCurTime;
            dwDeltaTime = lgTempTime.LowPart;
        }

        /* 更新整个链表中的最小等待时间 */
        dwRetTime = (dwRetTime < dwDeltaTime ? dwRetTime : dwDeltaTime);

        ppstTimerEntry = ppstNextTimerEntry;
    }

    /* 为了防止定时器执行过程中进行定时器的删除，这边将kill标记进行复位 */
    if ((HANDLE) NULL != gshOsTimeKillEvent)
    {
        if (0 == ResetEvent(gshOsTimeKillEvent))
        {
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeCallback: Failed to reset gshOsTimeKillEvent event to stop timer running thread! ErrorCode:%u\n",
                    GetLastError());*/
        }
    }
    LeaveCriticalSection(&gsOsTime_cs);

    /* 执行定时时间到的回调函数 */
    while (lIndex > 0)
    {
        OSTIME_TriggerCallBack(&gspstTimersArray[--lIndex]);
    }

    /* 同时同步退出线程的执行 */
    if ((HANDLE) NULL != gshOsTimeKillEvent)
    {
        if (0 == SetEvent(gshOsTimeKillEvent))
        {
            /*dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeCallback: Failed to set gshOsTimeKillEvent event to stop timer running thread! ErrorCode:%u\n",
                    GetLastError());*/
        }
    }

    /* 当程序执行完成后，将函数执行的时间更新到当前的定时器列表中，使得下次命中的时间更加的精确 */
    EnterCriticalSection(&gsGetSystemTime_cs);
    lgTempTime.QuadPart = OsGetTickCount64() - llCurTime;
    LeaveCriticalSection(&gsGetSystemTime_cs);

    dwAdjustTime = lgTempTime.LowPart;
    if (dwAdjustTime > dwRetTime)
    {
        dwRetTime = 0;
    }
    else
    {
        dwRetTime -= dwAdjustTime;
    }

    /* 将下次中断时间间隔返回    */
    return(dwRetTime/OSTIME_PREC);
}

/**
* 线程启动回调函数，该函数有定时器线程调用
* @param [IN] arg 没有使用
* @return 返回值为window线程启动函数固有。
*/
static DWORD CALLBACK OSTIME_SysTimeThread(IN LPVOID arg)
{
    DWORD dwSleepTime = 0;  /* 用于记录当前定时器队列中最小的等待时间 */
    DWORD dwRetVal = 0;

    IMOS_UNUSED_ARG(arg);

    while (FALSE == gsbOsTimeToDie)
    {
        dwSleepTime = OSTIME_SysTimeCallback();

        if (0 == dwSleepTime)
        {
            continue;
        }

        dwRetVal = WaitForSingleObject(gshOsTimeWakeEvent, dwSleepTime);
        if ((WAIT_TIMEOUT != dwRetVal) && (WAIT_OBJECT_0 != dwRetVal))
        {
            /* 定时器线程运行出现了异常，现在需要进行退出 */
     /*       dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeThread: The thread ID %u will exit with Calling WaitForSigleObject function! ErrorCode:%u\n",
                    gshOsTimeMMTimer, GetLastError());*/
            break;
        }
    }

    /* 释放定时器回调函数(OSTIME_SysTimeCallback)的缓存数组内存 */
    if (NULL != gspstTimersArray)
    {
        HeapFree(gshOSTimeHeap, 0, gspstTimersArray);
        gspstTimersArray = NULL;
    }

    /* 在数组内存清空时需要将长度标记为进行情况 */
    gslSizeLpTimers = 0;

    return 0;
}

/**
* 定时器内部线程运行启动函数,在启动定时前必须调用的函数。不支持多线程调用，必须同OSTIME_TimeStop配套使用
* @param [IN] 无
* @return OSTIME_NO_ERR 返回成功
*         OSTIME_ERR    返回失败
*/
unsigned int OSTIME_TimeStart(VOID)
{
    /* 判断当前是否已经启动了定时器线程，没有启动则启动定时器线程 */
    if ((HANDLE) NULL == gshOsTimeMMTimer)
    {
        gspstOsTimersList = NULL;
        gshOsTimeWakeEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        if ((HANDLE) NULL == gshOsTimeWakeEvent)
        {
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStart: Failed to create an Time wake Event for pstTimerEntry running! ErrorCode:%u\n",
                    GetLastError());*/
            return OSTIME_ERR;
        }

        gsbOsTimeToDie = FALSE;

        /* 创建一个堆供定时器模块使用 */
        gshOSTimeHeap = HeapCreate(0, 0, 0);
        if ((HANDLE) NULL == gshOSTimeHeap)
        {
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStart: Failed to create an Time Heap for timer running need! ErrorCode:%u\n",
                    GetLastError());*/
            CloseHandle(gshOsTimeWakeEvent);
            gshOsTimeWakeEvent = (HANDLE) NULL;
            return OSTIME_ERR;
        }
        gshOsTimeMMTimer = CreateThread(NULL, 0, OSTIME_SysTimeThread, NULL, 0, NULL);
        if ((HANDLE) NULL == gshOsTimeMMTimer)
        {
        /*    dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStart: Failed to create an Time thread for pstTimerEntry running! ErrorCode:%u\n",
                    GetLastError());*/
            CloseHandle(gshOsTimeWakeEvent);
            gshOsTimeWakeEvent = (HANDLE) NULL;
            HeapDestroy(gshOSTimeHeap);
            gshOSTimeHeap = (HANDLE) NULL;

            return OSTIME_ERR;
        }

        InitializeCriticalSection(&gsOsTime_cs);
        InitializeCriticalSection(&gsOSTimerID_cs);
        InitializeCriticalSection(&gsGetSystemTime_cs);

        /* 对返回错误不进行特别处理，只是定时器线程的优先级会降低一点 */
        if (0 == SetThreadPriority(gshOsTimeMMTimer, THREAD_PRIORITY_TIME_CRITICAL))
        {
           /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStart: Failed to set pstTimerEntry thread Priorty!ErrorCode:%u!\n",
                    GetLastError());*/
        }
        /*dsp_log(MOD_OSTIME, DSP_LOG_TRACE,
                "[TRACE] OSTIME_TimeStart: Have create an pstTimerEntry thread for pstTimerEntry running! The thread ID %u, The handle of timer heap is %u\n",
                gshOsTimeMMTimer, gshOSTimeHeap);*/

        gslgCurTime.QuadPart = 0;
    }
    return OSTIME_NO_ERR;
}

/**
* 定时器内部线程关闭函数，在退出时必须调用的函数.不支持多线程调用，必须同OSTIME_TimeStart配套使用
* @param [IN] 无
* @return 无
*/
VOID OSTIME_TimeStop(VOID)
{
    if ((HANDLE) NULL != gshOsTimeMMTimer)
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_TRACE,
                "[TRACE] OSTIME_TimeStop: Have stop an pstTimerEntry thread for pstTimerEntry running! The thread ID %u\n",
                gshOsTimeMMTimer);*/
        gsbOsTimeToDie = TRUE;
        if (0 == SetEvent(gshOsTimeWakeEvent))
        {
           /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStop: Failed to set gshOsTimeWakeEvent event to stop timer running thread! ErrorCode:%u\n",
                    GetLastError());*/
        }

        /* 等待定时器线程退出 */
        DWORD dwRetVal = WaitForSingleObject(gshOsTimeMMTimer, INFINITE);
        if ((WAIT_TIMEOUT != dwRetVal) && (WAIT_OBJECT_0 != dwRetVal))
        {
            /* 定时器线程运行出现了异常，现在需要进行退出 */
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStop: The thread ID %u will exit with Calling WaitForSigleObject function! ErrorCode:%u\n",
                    gshOsTimeMMTimer, GetLastError());*/
        }

        CloseHandle(gshOsTimeMMTimer);
        gshOsTimeMMTimer = (HANDLE) NULL;
        CloseHandle(gshOsTimeWakeEvent);
        gshOsTimeWakeEvent = (HANDLE) NULL;
        if ((HANDLE) NULL != gshOsTimeKillEvent)
        {
            CloseHandle(gshOsTimeKillEvent);
            gshOsTimeKillEvent = (HANDLE) NULL;
        }

        /* 增加错误判断，查看定时器调用函数外部是否调用匹配 */
        if ((NULL != gspstOsTimersList) || (0 != gsmapOsTimerID.size()))
        {
           /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStop: the OstimeSetEvent function is mistach with the OstimeKillEvent function call! the size of gsmapOsTimerID is %u\n",
                    gsmapOsTimerID.size());*/
        }

        gspstOsTimersList = NULL;
        DeleteCriticalSection(&gsOsTime_cs);
        DeleteCriticalSection(&gsOSTimerID_cs);
        DeleteCriticalSection(&gsGetSystemTime_cs);
        HeapDestroy(gshOSTimeHeap);
        gshOSTimeHeap = (HANDLE) NULL;

        /*将定时器的ID号也进行复位*/
        gsulCurTimerID = 0;
        gslgCurTime.QuadPart = 0;
    }
}


