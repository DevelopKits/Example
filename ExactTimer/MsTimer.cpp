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

/* ��ʱ������⺯���ӿ� */

/**
* ��ʱ���ڲ��߳�������������,��������ʱǰ������õĺ���,����ͬOSTIME_TimeStop����ʹ��
* @param [IN] ��
* @return OSTIME_NO_ERR ���سɹ�
*         OSTIME_ERR    ����ʧ��
*/

unsigned int MsTimer::OSTIME_TimeStart( VOID )
{
	 /* �жϵ�ǰ�Ƿ��Ѿ������˶�ʱ���̣߳�û��������������ʱ���߳� */
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

        /* ����һ���ѹ���ʱ��ģ��ʹ�� */
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

        /* �Է��ش��󲻽����ر���ֻ�Ƕ�ʱ���̵߳����ȼ��ή��һ�� */
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
* ��ʱ���ڲ��̹߳رպ��������˳�ʱ������õĺ���,����ͬOSTIME_TimeStart����ʹ��
* @param [IN] ��
* @return ��
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

        /* �ȴ���ʱ���߳��˳� */
        DWORD dwRetVal = WaitForSingleObject(m_hOsTimeMMTimer, INFINITE);
        if ((WAIT_TIMEOUT != dwRetVal) && (WAIT_OBJECT_0 != dwRetVal))
        {
            /* ��ʱ���߳����г������쳣��������Ҫ�����˳� */
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

        /* ���Ӵ����жϣ��鿴��ʱ�����ú����ⲿ�Ƿ����ƥ�� */
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

        /*����ʱ����ID��Ҳ���и�λ*/
        m_ulCurTimerID = 0;
        m_lgCurTime.QuadPart = 0;
    }

}
/**
* ����һ����ʱ��
* @param [IN] ulDelay ��ʱ�������ڣ���λms����Χ����Ϊ[1��65535].
* @param [IN] ulResol Ϊ����windows����ϵͳ�ĺ���������δ���á�
* @param [IN] lpFunc �ص�����������ΪLPOSTIMECALLBACK�ĺ���ָ��
* @param [IN] ulUser ���ݸ��ص������Ĳ�����
* @param [IN] ulFlags ��ʶ��ǰ��ʱ�������ͣ�����ֻ֧�����ֱ��:
              OSTIME_KILL_SYNCHRONOUS  ��ǰΪͬ����ʱ����OstimeKillEvent����ʱ���뱣֤���ٽ��лص���������
              OSTIME_PERIODIC �Ƿ���Ҫ���ڵ��ûص�������������������һ���Զ�ʱ��
* @return ��ʱ��ID�ţ����ʧ�ܣ��򷵻صĶ�ʱ��ID��Ϊ0���ɹ����ط�0ֵ
*/

unsigned int MsTimer::OstimeSetEvent( IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags )
{
	unsigned int ulFps = (1000/ulDelay);
	unsigned int msInterval = (1000 * OSTIME_PREC)/ulFps;

	/* ��33msΪ���ڵĶ�ʱ������΢��,���IPC 30֡ÿ��ʱ֡�ʲ������� */
	if (ulDelay == 33)
	{
		msInterval += OSTIME_33MS_ADJUST_TIME_VAL;
	}
	m_ulTimerID = OSTIME_SetEventInternal(msInterval, ulResol, lpFunc, ulUser, ulFlags);
	return m_ulTimerID;

}


/**
* ֹͣһ����ʱ��
* @param [IN] ulTimerID ��ʱ��ID�š�
* @return OSTIME_ERR ��ʾɾ����ʱ��ʧ��
*      OSTIME_NO_ERR ��ʾɾ����ʱ���ɹ�
*/
unsigned int MsTimer::OstimeKillEvent( IN unsigned int ulTimerID )
{
	 PSTOSTIMERENTRY  pstSelfTimerEntry = NULL;
    PSTOSTIMERENTRY  *ppstTimerEntry = NULL;

    EnterCriticalSection(&m_OsTime_cs);

    /* �����б�Ѱ�Ҷ�ʱ������ */
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
        /* ��ʾ�رն�ʱ��ʧ�� */
 /*       dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OstimeKillEvent: Cann't find OsTimer[TimerID: %u] in gspstOsTimersList\n",
                ulTimerID);*/
        return OSTIME_ERR;
    }

    /* ͬ���˳���Ҫ�ȴ��ص������������ */
    if (0 != (pstSelfTimerEntry->ulFlags & OSTIME_KILL_SYNCHRONOUS))
    {
        DWORD dwRetVal = WaitForSingleObject(m_hOsTimeKillEvent, INFINITE);
        if ((WAIT_TIMEOUT != dwRetVal) && (WAIT_OBJECT_0 != dwRetVal))
        {
            /* ��ʱ���߳����г������쳣��������Ҫ�����˳� */
          /*  dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OstimeKillEvent: The pstTimerEntry ID %u will exit with Calling WaitForSigleObject function! ErrorCode:%u\n",
                    ulTimerID, GetLastError());*/
        }
    }

    /* ��Ҫ�黹����Ķ�ʱ��ID�� */
    OsReleaseTimerID(pstSelfTimerEntry->ulTimerID);
    HeapFree(m_hOSTimeHeap, 0, pstSelfTimerEntry);
    pstSelfTimerEntry = NULL;

    return OSTIME_NO_ERR;
}

/**
* ��װwindows��ʱ��֧�ֻ��64λʱ����Ϣ��ע:�ú�����Ҫ�ⲿ��֤�����ж��̵߳���
* @param [IN] ��
* @return ��ǰ��ϵͳʱ�䣬���ص�Ϊ���ʱ�䣬����Ϊms��
*/
LONGLONG MsTimer::OsGetTickCount64( LPVOID arg )
{
	DWORD dwCurTime = timeGetTime();
	MsTimer* pUser =(MsTimer*) arg;
	/* �ж�ʱ���Ƿ������� */
	if (dwCurTime <pUser->m_lgCurTime.LowPart)
	{
		pUser->m_lgCurTime.HighPart++;
	}
	pUser->m_lgCurTime.LowPart = dwCurTime;
	return  pUser->m_lgCurTime.QuadPart*OSTIME_PREC;

}
/**
* �߳������ص��������ú����ж�ʱ���̵߳���
* @param [IN] arg û��ʹ��
* @return ����ֵΪwindow�߳������������С�
*/
DWORD CALLBACK MsTimer::OSTIME_SysTimeThread( IN LPVOID arg )
{
	 DWORD dwSleepTime = 0;  /* ���ڼ�¼��ǰ��ʱ����������С�ĵȴ�ʱ�� */
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
            /* ��ʱ���߳����г������쳣��������Ҫ�����˳� */
     /*       dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeThread: The thread ID %u will exit with Calling WaitForSigleObject function! ErrorCode:%u\n",
                    gshOsTimeMMTimer, GetLastError());*/
            break;
        }
    }

    /* �ͷŶ�ʱ���ص�����(OSTIME_SysTimeCallback)�Ļ��������ڴ� */
    if (NULL != pUser->m_pstTimersArray)
    {
        HeapFree(pUser->m_hOSTimeHeap, 0, pUser->m_pstTimersArray);
        pUser->m_pstTimersArray = NULL;
    }

    /* �������ڴ����ʱ��Ҫ�����ȱ��Ϊ������� */
    pUser->m_lSizeLpTimers = 0;

    return 0;

}

/**
* ��ʱ���̵߳�ִ�к������������̻߳ص���������
* @param [IN] ��
* @return ��ǰ��ʱ��������Ҫ�ȴ��ļ��ʱ��,��λΪms
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

    /* ��ʱ������Ϊ�գ�����ֱ���˳� */
    if (NULL == pUser->m_pstOsTimersList)
    {
        return(dwRetTime);
    }

    /* ��õ�ǰ��ϵͳʱ�䣬����ʱ��У��ʹ�� */
    EnterCriticalSection(&pUser->m_GetSystemTime_cs);
    llCurTime = OsGetTickCount64(arg);
    LeaveCriticalSection(&pUser->m_GetSystemTime_cs);

    /* ɨ�������б������µ�ʱ�����Ϣ������Ҫ�ص��Ķ�����õ�һ�������н��лص� */
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
                    /* ��ʶ��ǰ�Ķ�ʱ����ſռ��Ѿ����㣬��Ҫ���ж�̬���� */
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

            /* ���µ�ǰ����ʱ��Ϊ�´ζ�ʱ������ʱ��  */
            pstTimerEntry->llTriggerTime += pstTimerEntry->ulDelay;

            /* ��ʱ������Ϊ���ζ�ʱ�� */
            if (0 == (pstTimerEntry->ulFlags & OSTIME_PERIODIC))
            {
                /* �Ӷ�ʱ��������ɾ����ǰ�Ķ�ʱ�� */
                *ppstTimerEntry = *ppstNextTimerEntry;
                HeapFree(pUser->m_hOSTimeHeap, 0, pstTimerEntry);
                pstTimerEntry = NULL;

                /* ����ǰ��ʱ���ĵȴ�ʱ������Ϊ���� */
                dwDeltaTime = INFINITE;
            }
            else
            {
                /* �����´��е�ʱ���� */
                if (pstTimerEntry->llTriggerTime <= llCurTime)
                {
                    /* ����ʱ����Ҫ׷�ϵ�ʱ�������õ�ʱ����ʱ����Ҫ����ʱ��ʱ���ͬ��Ϊ��ǰϵͳʱ�� */
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
            /* �����ǰ�Ķ�ʱ��û�����У�ֱ���޸Ķ�ʱ���ڼ��� */
            lgTempTime.QuadPart = pstTimerEntry->llTriggerTime - llCurTime;
            dwDeltaTime = lgTempTime.LowPart;
        }

        /* �������������е���С�ȴ�ʱ�� */
        dwRetTime = (dwRetTime < dwDeltaTime ? dwRetTime : dwDeltaTime);

        ppstTimerEntry = ppstNextTimerEntry;
    }

    /* Ϊ�˷�ֹ��ʱ��ִ�й����н��ж�ʱ����ɾ������߽�kill��ǽ��и�λ */
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

    /* ִ�ж�ʱʱ�䵽�Ļص����� */
    while (lIndex > 0)
    {
        OSTIME_TriggerCallBack(&pUser->m_pstTimersArray[--lIndex]);
    }

    /* ͬʱͬ���˳��̵߳�ִ�� */
    if ((HANDLE) NULL != pUser->m_hOsTimeKillEvent)
    {
        if (0 == SetEvent(pUser->m_hOsTimeKillEvent))
        {
            /*dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeCallback: Failed to set gshOsTimeKillEvent event to stop timer running thread! ErrorCode:%u\n",
                    GetLastError());*/
        }
    }

    /* ������ִ����ɺ󣬽�����ִ�е�ʱ����µ���ǰ�Ķ�ʱ���б��У�ʹ���´����е�ʱ����ӵľ�ȷ */
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

    /* ���´��ж�ʱ��������    */
    return(dwRetTime/OSTIME_PREC);

}

/**
* �û�ע�����ʱ���Ķ�ʱ���ص�����
* @param [IN] lpTimer ��ʱ�����ƿ�ĵ�ַ
* @return ��
*/
VOID MsTimer::OSTIME_TriggerCallBack( IN PSTOSTIMERENTRY lpTimer )
{
	MsTimer* puser = (MsTimer*)(lpTimer->arg);
	/* �����û��趨�Ķ�ʱ��flag���λȷ���Ƿ񽫽ṹ�崫�ݸ��û� */
    if (lpTimer->ulFlags & OSTIME_ADJUST_PERIODIC)
    {
        STOSTIMERENTRY stOsTimerEntry = *lpTimer;
        (lpTimer->lpFunc)(lpTimer->ulTimerID, 0, lpTimer->ulUser, 0, (unsigned int) lpTimer);

        /* �û��޸��˶�ʱ�����ں���Ҫ���¶�ʱ�����ƿ� */
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
* ����һ����ʱ������ʱ��ģ���ڲ�ʵ�ֺ���
* @param [IN] ulDelay ��ʱ�������ڣ���λms����Χ����Ϊ�[1��65535].
* @param [IN] ulResol Ϊ����windows����ϵͳ�ĺ���������δ���á�
* @param [IN] lpFunc �ص�����������ΪLPOSTIMECALLBACK�ĺ���ָ��
* @param [IN] ulUser ���ݸ��ص������Ĳ�����
* @param [IN] ulFlags ��ʶ��ǰ��ʱ�������ͣ�����ֻ֧�����ֱ��:
              OSTIME_KILL_SYNCHRONOUS  ��ǰΪͬ����ʱ����OstimeKillEvent����ʱ���뱣֤���ٽ��лص���������
              OSTIME_PERIODIC �Ƿ���Ҫ���ڵ��ûص�������������������һ���Զ�ʱ��
* @return ��ʱ��ID�ţ����ʧ�ܣ��򷵻صĶ�ʱ��ID��Ϊ0���ɹ����ط�0ֵ
*/
unsigned int MsTimer::OSTIME_SetEventInternal( IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags )
{
	 /* �ж϶�ʱ�����ڵ���Ч�� */
    if ((ulDelay < OSSYSTIME_MININTERVAL) || (ulDelay > OSSYSTIME_MAXINTERVAL) || (NULL == lpFunc))
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OSTIME_SetEventInternal: The ulDelay paremeter in OsTimer is invalid\n");*/
        return 0;
    }

    /* ����һ�鶨ʱ�����ƿ� */
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
    /* �ж��Ƿ񴴽�ͬ����ʱ�������ʱͬ����ʱ������ֹͣ�¼�û�д���������¼����� */
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

    /* ����һ�ζ�ʱ�ص��������� */
    if (0 == SetEvent(m_hOsTimeWakeEvent))
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_WARNING,
                "[WARNING] OSTIME_SetEventInternal: Failed to set gshOsTimeWakeEvent event to the timer running thread! ErrorCode:%u\n",
                GetLastError());*/
    }

    return pstNewTimer->ulTimerID;
}

/**
* ʹ��C++��map����һ����ʱ��ID��
* @param [IN] ��
* @return ���ص�ǰ�Ķ�ʱ��ID��0 ��ʾ��ȡ��ʱ��IDʧ��
*/
unsigned int MsTimer::OsGenTimerID( VOID )
{
	/*��ѯ�������Ƿ��Ѿ����ж�ʱ��ID��,��ʱ��ID����Ϊ0,ͬʱ���Բ�ѯ�Ĵ���ΪOSTOTAL_TIMERID_TRYNUM��*/
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
			/* ����õĶ�ʱ��Timerѹ��map���� */
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
* ɾ��ȫ�ֶ����е�timerID���
* @param [IN] ulTimerID��Ҫɾ���Ķ�ʱ��ID��
* @return ��
*/
VOID MsTimer::OsReleaseTimerID( IN unsigned int ulTimerID )
{
	EnterCriticalSection(&m_OSTimerID_cs);
	m_mapOsTimerID.erase(ulTimerID);
	LeaveCriticalSection(&m_OSTimerID_cs);
	return;

}
