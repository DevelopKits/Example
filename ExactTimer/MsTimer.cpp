#include "MsTimer.h"

#pragma comment(lib,"winmm.lib")

MsTimer::MsTimer(void)
{
	m_hOsTimeMMTimer = NULL;
	m_pstOsTimersList = NULL;
	m_pstTimersArray = NULL;
	m_lSizeLpTimers = 0;
	m_hOsTimeKillEvent = NULL;
	m_hOsTimeWakeEvent = NULL;
	m_bOsTimeToDie = TRUE;
	m_hOSTimeHeap = NULL;
	m_ulCurTimerID = 0;
	m_ulTimerID = -1;
}

MsTimer::~MsTimer(void)
{
}

/* 定时器库对外函数接口 */

/**
* 定时器内部线程运行启动函数,在启动定时前必须调用的函数,必须同OSTIME_TimeStop配套使用
* @param [IN] 无
* @return OSTIME_NO_ERR 返回成功
*         OSTIME_ERR    返回失败
*/

unsigned int MsTimer::OSTIME_TimeStart( VOID )
{
	 /* 判断当前是否已经启动了定时器线程，没有启动则启动定时器线程 */
    if ((HANDLE) NULL == m_hOsTimeMMTimer)
    {
        m_pstOsTimersList = NULL;
        m_hOsTimeWakeEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        if ((HANDLE) NULL == m_hOsTimeWakeEvent)
        {
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStart: Failed to create an Time wake Event for pstTimerEntry running! ErrorCode:%u\n",
                    GetLastError());*/
            return OSTIME_ERR;
        }

        m_bOsTimeToDie = FALSE;

        /* 创建一个堆供定时器模块使用 */
        m_hOSTimeHeap = HeapCreate(0, 0, 0);
        if ((HANDLE) NULL == m_hOSTimeHeap)
        {
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStart: Failed to create an Time Heap for timer running need! ErrorCode:%u\n",
                    GetLastError());*/
            CloseHandle(m_hOsTimeWakeEvent);
            m_hOsTimeWakeEvent = (HANDLE) NULL;
            return OSTIME_ERR;
        }
        m_hOsTimeMMTimer = CreateThread(NULL, 0, OSTIME_SysTimeThread, this, 0, NULL);
        if ((HANDLE) NULL == m_hOsTimeMMTimer)
        {
        /*    dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStart: Failed to create an Time thread for pstTimerEntry running! ErrorCode:%u\n",
                    GetLastError());*/
            CloseHandle(m_hOsTimeWakeEvent);
            m_hOsTimeWakeEvent = (HANDLE) NULL;
            HeapDestroy(m_hOSTimeHeap);
            m_hOSTimeHeap = (HANDLE) NULL;

            return OSTIME_ERR;
        }

        InitializeCriticalSection(&m_OsTime_cs);
        InitializeCriticalSection(&m_OSTimerID_cs);
        InitializeCriticalSection(&m_GetSystemTime_cs);

        /* 对返回错误不进行特别处理，只是定时器线程的优先级会降低一点 */
        if (0 == SetThreadPriority(m_hOsTimeMMTimer, THREAD_PRIORITY_TIME_CRITICAL))
        {
           /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStart: Failed to set pstTimerEntry thread Priorty!ErrorCode:%u!\n",
                    GetLastError());*/
        }
        /*dsp_log(MOD_OSTIME, DSP_LOG_TRACE,
                "[TRACE] OSTIME_TimeStart: Have create an pstTimerEntry thread for pstTimerEntry running! The thread ID %u, The handle of timer heap is %u\n",
                gshOsTimeMMTimer, gshOSTimeHeap);*/

        m_lgCurTime.QuadPart = 0;
    }
    return OSTIME_NO_ERR;

}

/**
* 定时器内部线程关闭函数，在退出时必须调用的函数,必须同OSTIME_TimeStart配套使用
* @param [IN] 无
* @return 无
*/
VOID MsTimer::OSTIME_TimeStop(VOID)
{
	OstimeKillEvent(m_ulTimerID);
	if ((HANDLE) NULL != m_hOsTimeMMTimer)
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_TRACE,
                "[TRACE] OSTIME_TimeStop: Have stop an pstTimerEntry thread for pstTimerEntry running! The thread ID %u\n",
                gshOsTimeMMTimer);*/
        m_bOsTimeToDie = TRUE;
        if (0 == SetEvent(m_hOsTimeWakeEvent))
        {
           /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStop: Failed to set gshOsTimeWakeEvent event to stop timer running thread! ErrorCode:%u\n",
                    GetLastError());*/
        }

        /* 等待定时器线程退出 */
        DWORD dwRetVal = WaitForSingleObject(m_hOsTimeMMTimer, INFINITE);
        if ((WAIT_TIMEOUT != dwRetVal) && (WAIT_OBJECT_0 != dwRetVal))
        {
            /* 定时器线程运行出现了异常，现在需要进行退出 */
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStop: The thread ID %u will exit with Calling WaitForSigleObject function! ErrorCode:%u\n",
                    gshOsTimeMMTimer, GetLastError());*/
        }

        CloseHandle(m_hOsTimeMMTimer);
        m_hOsTimeMMTimer = (HANDLE) NULL;
        CloseHandle(m_hOsTimeWakeEvent);
        m_hOsTimeWakeEvent = (HANDLE) NULL;
        if ((HANDLE) NULL != m_hOsTimeKillEvent)
        {
            CloseHandle(m_hOsTimeKillEvent);
            m_hOsTimeKillEvent = (HANDLE) NULL;
        }

        /* 增加错误判断，查看定时器调用函数外部是否调用匹配 */
        if ((NULL != m_pstOsTimersList) || (0 != m_mapOsTimerID.size()))
        {
           /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_TimeStop: the OstimeSetEvent function is mistach with the OstimeKillEvent function call! the size of gsmapOsTimerID is %u\n",
                    gsmapOsTimerID.size());*/
        }

        m_pstOsTimersList = NULL;
        DeleteCriticalSection(&m_OsTime_cs);
        DeleteCriticalSection(&m_OSTimerID_cs);
        DeleteCriticalSection(&m_GetSystemTime_cs);
        HeapDestroy(m_hOSTimeHeap);
        m_hOSTimeHeap = (HANDLE) NULL;

        /*将定时器的ID号也进行复位*/
        m_ulCurTimerID = 0;
        m_lgCurTime.QuadPart = 0;
    }

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

unsigned int MsTimer::OstimeSetEvent( IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags )
{
	unsigned int ulFps = (1000/ulDelay);
	unsigned int msInterval = (1000 * OSTIME_PREC)/ulFps;

	/* 对33ms为周期的定时器进行微调,规避IPC 30帧每秒时帧率不足问题 */
	if (ulDelay == 33)
	{
		msInterval += OSTIME_33MS_ADJUST_TIME_VAL;
	}
	m_ulTimerID = OSTIME_SetEventInternal(msInterval, ulResol, lpFunc, ulUser, ulFlags);
	return m_ulTimerID;

}


/**
* 停止一个定时器
* @param [IN] ulTimerID 定时器ID号。
* @return OSTIME_ERR 表示删除定时器失败
*      OSTIME_NO_ERR 表示删除定时器成功
*/
unsigned int MsTimer::OstimeKillEvent( IN unsigned int ulTimerID )
{
	 PSTOSTIMERENTRY  pstSelfTimerEntry = NULL;
    PSTOSTIMERENTRY  *ppstTimerEntry = NULL;

    EnterCriticalSection(&m_OsTime_cs);

    /* 遍历列表寻找定时器对象 */
    for (ppstTimerEntry = &m_pstOsTimersList; *ppstTimerEntry; ppstTimerEntry = &(*ppstTimerEntry)->pstNextOsTimerEntry)
    {
        if (ulTimerID == (*ppstTimerEntry)->ulTimerID)
        {
            pstSelfTimerEntry = *ppstTimerEntry;
            *ppstTimerEntry = (*ppstTimerEntry)->pstNextOsTimerEntry;
            break;
        }
    }
    LeaveCriticalSection(&m_OsTime_cs);

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
        DWORD dwRetVal = WaitForSingleObject(m_hOsTimeKillEvent, INFINITE);
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
    HeapFree(m_hOSTimeHeap, 0, pstSelfTimerEntry);
    pstSelfTimerEntry = NULL;

    return OSTIME_NO_ERR;
}

/**
* 包装windows定时器支持获得64位时间信息，注:该函数需要外部保证不进行多线程调用
* @param [IN] 无
* @return 当前的系统时间，返回的为相对时间，精度为ms，
*/
LONGLONG MsTimer::OsGetTickCount64( LPVOID arg )
{
	DWORD dwCurTime = timeGetTime();
	MsTimer* pUser =(MsTimer*) arg;
	/* 判断时间是否产生溢出 */
	if (dwCurTime <pUser->m_lgCurTime.LowPart)
	{
		pUser->m_lgCurTime.HighPart++;
	}
	pUser->m_lgCurTime.LowPart = dwCurTime;
	return  pUser->m_lgCurTime.QuadPart*OSTIME_PREC;

}
/**
* 线程启动回调函数，该函数有定时器线程调用
* @param [IN] arg 没有使用
* @return 返回值为window线程启动函数固有。
*/
DWORD CALLBACK MsTimer::OSTIME_SysTimeThread( IN LPVOID arg )
{
	 DWORD dwSleepTime = 0;  /* 用于记录当前定时器队列中最小的等待时间 */
    DWORD dwRetVal = 0;

    IMOS_UNUSED_ARG(arg);
	MsTimer* pUser =(MsTimer*) arg;

    while (FALSE == pUser->m_bOsTimeToDie)
    {
        dwSleepTime = OSTIME_SysTimeCallback(arg);

        if (0 == dwSleepTime)
        {
            continue;
        }

        dwRetVal = WaitForSingleObject(pUser->m_hOsTimeWakeEvent, dwSleepTime);
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
    if (NULL != pUser->m_pstTimersArray)
    {
        HeapFree(pUser->m_hOSTimeHeap, 0, pUser->m_pstTimersArray);
        pUser->m_pstTimersArray = NULL;
    }

    /* 在数组内存清空时需要将长度标记为进行情况 */
    pUser->m_lSizeLpTimers = 0;

    return 0;

}

/**
* 定时器线程的执行函数，被上面线程回调函数调用
* @param [IN] 无
* @return 当前定时器链表需要等待的间隔时间,单位为ms
*/
unsigned int MsTimer::OSTIME_SysTimeCallback( LPVOID arg )
{
	MsTimer* pUser =(MsTimer*) arg;
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
    if (NULL == pUser->m_pstOsTimersList)
    {
        return(dwRetTime);
    }

    /* 获得当前的系统时间，进行时间校正使用 */
    EnterCriticalSection(&pUser->m_GetSystemTime_cs);
    llCurTime = OsGetTickCount64(arg);
    LeaveCriticalSection(&pUser->m_GetSystemTime_cs);

    /* 扫描整个列表，计算新的时间戳信息，将需要回调的对象放置到一个数组中进行回调 */
    EnterCriticalSection(&pUser->m_OsTime_cs);
    for (ppstTimerEntry = &pUser->m_pstOsTimersList; *ppstTimerEntry != NULL; )
    {
        pstTimerEntry = *ppstTimerEntry;
        ppstNextTimerEntry = &pstTimerEntry->pstNextOsTimerEntry;
        if (llCurTime >= pstTimerEntry->llTriggerTime)
        {
            if (NULL != pstTimerEntry->lpFunc)
            {
                if (lIndex == pUser->m_lSizeLpTimers)
                {
                    /* 标识当前的定时器存放空间已经不足，需要进行动态申请 */
                    if (NULL != pUser->m_pstTimersArray)
                    {
                       pUser->m_pstTimersArray = (PSTOSTIMERENTRY) HeapReAlloc(pUser->m_hOSTimeHeap, 0, pUser->m_pstTimersArray, ((DWORD)(++pUser->m_lSizeLpTimers)) * sizeof(STOSTIMERENTRY));
                        if (NULL == pUser->m_pstTimersArray)
                        {/*
                            dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                                    "[ERROR] OSTIME_SysTimeCallback: Failed to call HeapReAlloc with %u Bytes\n",
                                    (DWORD) gslSizeLpTimers * sizeof(STOSTIMERENTRY));*/
                            lIndex = 0;
                            pUser->m_lSizeLpTimers = 0;
                            break;
                        }
                    }
                    else
                    {
                        pUser->m_pstTimersArray = (PSTOSTIMERENTRY) HeapAlloc(pUser->m_hOSTimeHeap, 0, ((DWORD)(++pUser->m_lSizeLpTimers)) * sizeof(STOSTIMERENTRY));
                        if (NULL == pUser->m_pstTimersArray)
                        {
                         /*   dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                                    "[ERROR] OSTIME_SysTimeCallback: Failed to call HeapAlloc with %d Bytes\n",
                                    (DWORD) gslSizeLpTimers * sizeof(STOSTIMERENTRY));*/
                            lIndex = 0;
                            pUser->m_lSizeLpTimers = 0;
                            break;
                        }
                    }
                }
                pUser->m_pstTimersArray[lIndex++] = *pstTimerEntry;

            }

            /* 更新当前触发时间为下次定时器到的时间  */
            pstTimerEntry->llTriggerTime += pstTimerEntry->ulDelay;

            /* 定时器设置为单次定时器 */
            if (0 == (pstTimerEntry->ulFlags & OSTIME_PERIODIC))
            {
                /* 从定时器链表中删除当前的定时器 */
                *ppstTimerEntry = *ppstNextTimerEntry;
                HeapFree(pUser->m_hOSTimeHeap, 0, pstTimerEntry);
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
    if ((HANDLE) NULL != pUser->m_hOsTimeKillEvent)
    {
        if (0 == ResetEvent(pUser->m_hOsTimeKillEvent))
        {
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeCallback: Failed to reset gshOsTimeKillEvent event to stop timer running thread! ErrorCode:%u\n",
                    GetLastError());*/
        }
    }
    LeaveCriticalSection(&pUser->m_OsTime_cs);

    /* 执行定时时间到的回调函数 */
    while (lIndex > 0)
    {
        OSTIME_TriggerCallBack(&pUser->m_pstTimersArray[--lIndex]);
    }

    /* 同时同步退出线程的执行 */
    if ((HANDLE) NULL != pUser->m_hOsTimeKillEvent)
    {
        if (0 == SetEvent(pUser->m_hOsTimeKillEvent))
        {
            /*dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeCallback: Failed to set gshOsTimeKillEvent event to stop timer running thread! ErrorCode:%u\n",
                    GetLastError());*/
        }
    }

    /* 当程序执行完成后，将函数执行的时间更新到当前的定时器列表中，使得下次命中的时间更加的精确 */
    EnterCriticalSection(&pUser->m_GetSystemTime_cs);
    lgTempTime.QuadPart = OsGetTickCount64(arg) - llCurTime;
    LeaveCriticalSection(&pUser->m_GetSystemTime_cs);

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
* 用户注册给定时器的定时器回调函数
* @param [IN] lpTimer 定时器控制块的地址
* @return 无
*/
VOID MsTimer::OSTIME_TriggerCallBack( IN PSTOSTIMERENTRY lpTimer )
{
	MsTimer* puser = (MsTimer*)(lpTimer->arg);
	/* 根据用户设定的定时器flag标记位确认是否将结构体传递给用户 */
    if (lpTimer->ulFlags & OSTIME_ADJUST_PERIODIC)
    {
        STOSTIMERENTRY stOsTimerEntry = *lpTimer;
        (lpTimer->lpFunc)(lpTimer->ulTimerID, 0, lpTimer->ulUser, 0, (unsigned int) lpTimer);

        /* 用户修改了定时器周期后，需要更新定时器控制块 */
        if (stOsTimerEntry.ulDelay != lpTimer->ulDelay)
        {
            EnterCriticalSection(&puser->m_GetSystemTime_cs);
            for (PSTOSTIMERENTRY pStOsTimerEntry = puser->m_pstOsTimersList; pStOsTimerEntry != NULL; pStOsTimerEntry = pStOsTimerEntry->pstNextOsTimerEntry)
            {
                if (pStOsTimerEntry->ulTimerID == lpTimer->ulTimerID)
                {
                   /* dsp_log(MOD_OSTIME, DSP_LOG_TRACE,
                            "[INIFO] %d OSTIME_SysTimeCallback: change from %d to %d\n",
                            pStOsTimerEntry->ulTimerID, pStOsTimerEntry->ulDelay, lpTimer->ulDelay);*/
                    pStOsTimerEntry->ulDelay = lpTimer->ulDelay;
                }
            }
            LeaveCriticalSection(&puser->m_GetSystemTime_cs);
        }
    }
    else
    {
        (lpTimer->lpFunc)(lpTimer->ulTimerID, 0, lpTimer->ulUser, 0, 0);
    }
}

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
unsigned int MsTimer::OSTIME_SetEventInternal( IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags )
{
	 /* 判断定时器周期的有效性 */
    if ((ulDelay < OSSYSTIME_MININTERVAL) || (ulDelay > OSSYSTIME_MAXINTERVAL) || (NULL == lpFunc))
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OSTIME_SetEventInternal: The ulDelay paremeter in OsTimer is invalid\n");*/
        return 0;
    }

    /* 申请一块定时器控制块 */
    PSTOSTIMERENTRY pstNewTimer = (PSTOSTIMERENTRY) HeapAlloc(m_hOSTimeHeap, 0, sizeof(STOSTIMERENTRY));
    if (NULL == pstNewTimer)
    {
      /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OSTIME_SetEventInternal: Cann't HeapAlloc a memory for pstTimerEntry contrl\n");*/
        return 0;
    }

    pstNewTimer->ulDelay = ulDelay;

    EnterCriticalSection(&m_GetSystemTime_cs);
    pstNewTimer->llTriggerTime = OsGetTickCount64(this) + ulDelay;
    LeaveCriticalSection(&m_GetSystemTime_cs);

    pstNewTimer->ulResol = ulResol;
    pstNewTimer->lpFunc = lpFunc;
    pstNewTimer->ulUser = ulUser;
    pstNewTimer->ulFlags = ulFlags;
	pstNewTimer->arg = this;

    EnterCriticalSection(&m_OsTime_cs);
    /* 判断是否创建同步定时器，如果时同步定时器并且停止事件没有创建则进行事件创建 */
    if ((0 != (ulFlags & OSTIME_KILL_SYNCHRONOUS)) && ((HANDLE) NULL == m_hOsTimeKillEvent))
    {
        m_hOsTimeKillEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
        if ((HANDLE) NULL == m_hOsTimeKillEvent)
        {
           /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SetEventInternal: Failed to create an Time kill Event for pstTimerEntry running! ErrorCode:%u\n",
                    GetLastError());*/
            LeaveCriticalSection(&m_OsTime_cs);
            HeapFree(m_hOSTimeHeap, 0, pstNewTimer);
            pstNewTimer = NULL;

            return 0;
        }
    }

    pstNewTimer->ulTimerID = OsGenTimerID();
    if (0 == pstNewTimer->ulTimerID)
    {
        LeaveCriticalSection(&m_OsTime_cs);
        HeapFree(m_hOSTimeHeap, 0, pstNewTimer);
        pstNewTimer = NULL;
     /*   dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OSTIME_SetEventInternal: Cann't general an timerID for pstTimerEntry contrl\n");*/
        return 0;
    }
    pstNewTimer->pstNextOsTimerEntry = m_pstOsTimersList;
    m_pstOsTimersList = pstNewTimer;
    LeaveCriticalSection(&m_OsTime_cs);

    /* 触发一次定时回调函数调用 */
    if (0 == SetEvent(m_hOsTimeWakeEvent))
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_WARNING,
                "[WARNING] OSTIME_SetEventInternal: Failed to set gshOsTimeWakeEvent event to the timer running thread! ErrorCode:%u\n",
                GetLastError());*/
    }

    return pstNewTimer->ulTimerID;
}

/**
* 使用C++的map产生一个定时器ID号
* @param [IN] 无
* @return 返回当前的定时器ID。0 表示获取定时器ID失败
*/
unsigned int MsTimer::OsGenTimerID( VOID )
{
	/*查询队列中是否已经含有定时器ID号,定时器ID不能为0,同时尝试查询的次数为OSTOTAL_TIMERID_TRYNUM次*/
	std::map<unsigned int, unsigned int>::iterator iter;

	EnterCriticalSection(&m_OSTimerID_cs);
	m_ulCurTimerID++;
	if (0 == m_ulCurTimerID)
	{
		m_ulCurTimerID++;
	}

	for (unsigned int ulIndex = 0; ulIndex < OSTOTAL_TIMERID_TRYNUM; ulIndex++)
	{
		iter = m_mapOsTimerID.find(m_ulCurTimerID);
		if (m_mapOsTimerID.end() != iter)
		{
			m_ulCurTimerID++;
			if (0 == m_ulCurTimerID)
			{
				m_ulCurTimerID++;
			}
		}
		else
		{
			/* 将获得的定时器Timer压入map队列 */
			m_mapOsTimerID.insert(std::pair<unsigned int, unsigned int>(m_ulCurTimerID, m_ulCurTimerID));

			unsigned int ulTimerIDTmp = m_ulCurTimerID;
			LeaveCriticalSection(&m_OSTimerID_cs);
			return ulTimerIDTmp;
		}
	}
	LeaveCriticalSection(&m_OSTimerID_cs);
	return 0;
}
/**
* 删除全局队列中的timerID编号
* @param [IN] ulTimerID需要删除的定时器ID号
* @return 无
*/
VOID MsTimer::OsReleaseTimerID( IN unsigned int ulTimerID )
{
	EnterCriticalSection(&m_OSTimerID_cs);
	m_mapOsTimerID.erase(ulTimerID);
	LeaveCriticalSection(&m_OSTimerID_cs);
	return;

}
