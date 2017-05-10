/* C++��׼��ͷ�ļ� */
#include <list>
#include <map>
#include <windows.h>

/* ��ʱ����ͷ�ļ� */
#include "os_timer.h"


/* ��ʱ���Ķ�ʱ���ȷ�Χ */
#define OSSYSTIME_MININTERVAL (1)
#define OSSYSTIME_MAXINTERVAL (65535)
#define IMOS_UNUSED_ARG(x)    ((VOID) x)

/* ��ʱ����ȫ�ֱ��� */
static    HANDLE                 gshOsTimeMMTimer = (HANDLE) NULL;     /*��ʱ���߳̾��*/
static    PSTOSTIMERENTRY        gspstOsTimersList = NULL;             /*��ʱ�����ƶ���*/
static    PSTOSTIMERENTRY        gspstTimersArray = NULL;              /*���ڱ���ص�����������*/
static    int                    gslSizeLpTimers = 0;                  /*��ʱ���ص�����������ڴ泤��*/
static    HANDLE                 gshOsTimeKillEvent = (HANDLE) NULL;   /*��ʱ��ͬ���˳����*/
static    HANDLE                 gshOsTimeWakeEvent = (HANDLE) NULL;   /*��ʱ���̴߳����¼�*/
static    BOOL                   gsbOsTimeToDie = TRUE;                /*��ʱ���߳��˳����*/
static    HANDLE                 gshOSTimeHeap = (HANDLE) NULL;        /*���ڶ�ʱ��ģ���ڴ�����*/

/* ��TimerID�����йص�ȫ�ֱ��� */
static    std::map<unsigned int, unsigned int> gsmapOsTimerID;              /*��ʱ��IDά������*/
static    CRITICAL_SECTION       gsOSTimerID_cs;              /*��ʱ��IDά���ؼ���*/
static    CRITICAL_SECTION       gsGetSystemTime_cs;          /*��ʱ�����ʱ���ȡ�ؼ���*/
static    CRITICAL_SECTION       gsOsTime_cs;                 /*��ʱ�����ƿ�ά���ؼ���*/
static    LARGE_INTEGER          gslgCurTime;                 /*ϵͳ����������ʱ��*/
static    unsigned int           gsulCurTimerID = 0;          /*��һ��ȫ�ֵĶ�ʱ��ID�� */


/**
* ��װwindows��ʱ��֧�ֻ��64λʱ����Ϣ��ע:�ú�����Ҫ�ⲿ��֤�����ж��̵߳���
* @param [IN] ��
* @return ��ǰ��ϵͳʱ�䣬���ص�Ϊ���ʱ�䣬����Ϊms��
*/
LONGLONG  OsGetTickCount64(VOID)
{
	DWORD dwCurTime = timeGetTime();

	/* �ж�ʱ���Ƿ������� */
	if (dwCurTime < gslgCurTime.LowPart)
	{
		gslgCurTime.HighPart++;
	}
	gslgCurTime.LowPart = dwCurTime;
	return  gslgCurTime.QuadPart*OSTIME_PREC;
}


/**
* �߳������ص��������ú����ж�ʱ���̵߳���
* @param [IN] arg û��ʹ��
* @return ����ֵΪwindow�߳������������С�
*/
static DWORD CALLBACK OSTIME_SysTimeThread(IN LPVOID arg);

/**
* ��ʱ���̵߳�ִ�к������������̻߳ص���������
* @param [IN] ��
* @return ��ǰ��ʱ��������Ҫ�ȴ��ļ��ʱ��,��λΪms
*/
static unsigned int OSTIME_SysTimeCallback(VOID);

/**
* �û�ע�����ʱ���Ķ�ʱ���ص�����
* @param [IN] lpTimer ��ʱ�����ƿ�ĵ�ַ
* @return ��
*/
static VOID OSTIME_TriggerCallBack(IN PSTOSTIMERENTRY lpTimer);

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
unsigned int OSTIME_SetEventInternal(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags);


/**
* ʹ��C++��map����һ����ʱ��ID��
* @param [IN] ��
* @return ���ص�ǰ�Ķ�ʱ��ID��0 ��ʾ��ȡ��ʱ��IDʧ��
*/
unsigned int OsGenTimerID(VOID)
{
    /*��ѯ�������Ƿ��Ѿ����ж�ʱ��ID��,��ʱ��ID����Ϊ0,ͬʱ���Բ�ѯ�Ĵ���ΪOSTOTAL_TIMERID_TRYNUM��*/
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
            /* ����õĶ�ʱ��Timerѹ��map���� */
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
* ɾ��ȫ�ֶ����е�timerID���
* @param [IN] ulTimerID��Ҫɾ���Ķ�ʱ��ID��
* @return ��
*/
VOID OsReleaseTimerID(IN unsigned int ulTimerID)
{
    EnterCriticalSection(&gsOSTimerID_cs);
    gsmapOsTimerID.erase(ulTimerID);
    LeaveCriticalSection(&gsOSTimerID_cs);
    return;
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
unsigned int OstimeSetEvent(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags)
{
    unsigned int ulFps = (1000/ulDelay);
    unsigned int msInterval = (1000 * OSTIME_PREC)/ulFps;

    /* ��33msΪ���ڵĶ�ʱ������΢��,���IPC 30֡ÿ��ʱ֡�ʲ������� */
    if (ulDelay == 33)
    {
        msInterval += OSTIME_33MS_ADJUST_TIME_VAL;
    }

    return OSTIME_SetEventInternal(msInterval, ulResol, lpFunc, ulUser, ulFlags);
}

/**
* ֹͣһ����ʱ��
* @param [IN] ulTimerID ��ʱ��ID�š�
* @return OSTIME_ERR ��ʾɾ����ʱ��ʧ��
*      OSTIME_NO_ERR ��ʾɾ����ʱ���ɹ�
*/
unsigned int OstimeKillEvent(IN unsigned int ulTimerID)
{
    PSTOSTIMERENTRY  pstSelfTimerEntry = NULL;
    PSTOSTIMERENTRY  *ppstTimerEntry = NULL;

    EnterCriticalSection(&gsOsTime_cs);

    /* �����б�Ѱ�Ҷ�ʱ������ */
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
        /* ��ʾ�رն�ʱ��ʧ�� */
 /*       dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OstimeKillEvent: Cann't find OsTimer[TimerID: %u] in gspstOsTimersList\n",
                ulTimerID);*/
        return OSTIME_ERR;
    }

    /* ͬ���˳���Ҫ�ȴ��ص������������ */
    if (0 != (pstSelfTimerEntry->ulFlags & OSTIME_KILL_SYNCHRONOUS))
    {
        DWORD dwRetVal = WaitForSingleObject(gshOsTimeKillEvent, INFINITE);
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
    HeapFree(gshOSTimeHeap, 0, pstSelfTimerEntry);
    pstSelfTimerEntry = NULL;

    return OSTIME_NO_ERR;
}


/**
* ����һ����ʱ������ʱ��ģ���ڲ�ʵ�ֺ���
* @param [IN] ulDelay ��ʱ�������ڣ���λms����Χ����Ϊ[1��65535].
* @param [IN] ulResol Ϊ����windows����ϵͳ�ĺ���������δ���á�
* @param [IN] lpFunc �ص�����������ΪLPOSTIMECALLBACK�ĺ���ָ��
* @param [IN] ulUser ���ݸ��ص������Ĳ�����
* @param [IN] ulFlags ��ʶ��ǰ��ʱ�������ͣ�����ֻ֧�����ֱ��:
              OSTIME_KILL_SYNCHRONOUS  ��ǰΪͬ����ʱ����OstimeKillEvent����ʱ���뱣֤���ٽ��лص���������
              OSTIME_PERIODIC �Ƿ���Ҫ���ڵ��ûص�������������������һ���Զ�ʱ��
* @return ��ʱ��ID�ţ����ʧ�ܣ��򷵻صĶ�ʱ��ID��Ϊ0���ɹ����ط�0ֵ
*/
unsigned int OSTIME_SetEventInternal(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags)
{
    /* �ж϶�ʱ�����ڵ���Ч�� */
    if ((ulDelay < OSSYSTIME_MININTERVAL) || (ulDelay > OSSYSTIME_MAXINTERVAL) || (NULL == lpFunc))
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                "[ERROR] OSTIME_SetEventInternal: The ulDelay paremeter in OsTimer is invalid\n");*/
        return 0;
    }

    /* ����һ�鶨ʱ�����ƿ� */
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
    /* �ж��Ƿ񴴽�ͬ����ʱ�������ʱͬ����ʱ������ֹͣ�¼�û�д���������¼����� */
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

    /* ����һ�ζ�ʱ�ص��������� */
    if (0 == SetEvent(gshOsTimeWakeEvent))
    {
       /* dsp_log(MOD_OSTIME, DSP_LOG_WARNING,
                "[WARNING] OSTIME_SetEventInternal: Failed to set gshOsTimeWakeEvent event to the timer running thread! ErrorCode:%u\n",
                GetLastError());*/
    }

    return pstNewTimer->ulTimerID;
}

/**
* �û�ע�����ʱ���Ķ�ʱ���ص�����
* @param [IN] lpTimer ��ʱ�����ƿ�ĵ�ַ
* @return ��
*/
static VOID OSTIME_TriggerCallBack(IN PSTOSTIMERENTRY lpTimer)
{
    /* �����û��趨�Ķ�ʱ��flag���λȷ���Ƿ񽫽ṹ�崫�ݸ��û� */
    if (lpTimer->ulFlags & OSTIME_ADJUST_PERIODIC)
    {
        STOSTIMERENTRY stOsTimerEntry = *lpTimer;
        (lpTimer->lpFunc)(lpTimer->ulTimerID, 0, lpTimer->ulUser, 0, (unsigned int) lpTimer);

        /* �û��޸��˶�ʱ�����ں���Ҫ���¶�ʱ�����ƿ� */
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
* ��ʱ���̵߳�ִ�к������������̻߳ص���������
* @param [IN] ��
* @return ��ǰ��ʱ��������Ҫ�ȴ��ļ��ʱ��,��λΪms
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

    /* ��ʱ������Ϊ�գ�����ֱ���˳� */
    if (NULL == gspstOsTimersList)
    {
        return(dwRetTime);
    }

    /* ��õ�ǰ��ϵͳʱ�䣬����ʱ��У��ʹ�� */
    EnterCriticalSection(&gsGetSystemTime_cs);
    llCurTime = OsGetTickCount64();
    LeaveCriticalSection(&gsGetSystemTime_cs);

    /* ɨ�������б������µ�ʱ�����Ϣ������Ҫ�ص��Ķ�����õ�һ�������н��лص� */
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
                    /* ��ʶ��ǰ�Ķ�ʱ����ſռ��Ѿ����㣬��Ҫ���ж�̬���� */
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

            /* ���µ�ǰ����ʱ��Ϊ�´ζ�ʱ������ʱ��  */
            pstTimerEntry->llTriggerTime += pstTimerEntry->ulDelay;

            /* ��ʱ������Ϊ���ζ�ʱ�� */
            if (0 == (pstTimerEntry->ulFlags & OSTIME_PERIODIC))
            {
                /* �Ӷ�ʱ��������ɾ����ǰ�Ķ�ʱ�� */
                *ppstTimerEntry = *ppstNextTimerEntry;
                HeapFree(gshOSTimeHeap, 0, pstTimerEntry);
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

    /* ִ�ж�ʱʱ�䵽�Ļص����� */
    while (lIndex > 0)
    {
        OSTIME_TriggerCallBack(&gspstTimersArray[--lIndex]);
    }

    /* ͬʱͬ���˳��̵߳�ִ�� */
    if ((HANDLE) NULL != gshOsTimeKillEvent)
    {
        if (0 == SetEvent(gshOsTimeKillEvent))
        {
            /*dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeCallback: Failed to set gshOsTimeKillEvent event to stop timer running thread! ErrorCode:%u\n",
                    GetLastError());*/
        }
    }

    /* ������ִ����ɺ󣬽�����ִ�е�ʱ����µ���ǰ�Ķ�ʱ���б��У�ʹ���´����е�ʱ����ӵľ�ȷ */
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

    /* ���´��ж�ʱ��������    */
    return(dwRetTime/OSTIME_PREC);
}

/**
* �߳������ص��������ú����ж�ʱ���̵߳���
* @param [IN] arg û��ʹ��
* @return ����ֵΪwindow�߳������������С�
*/
static DWORD CALLBACK OSTIME_SysTimeThread(IN LPVOID arg)
{
    DWORD dwSleepTime = 0;  /* ���ڼ�¼��ǰ��ʱ����������С�ĵȴ�ʱ�� */
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
            /* ��ʱ���߳����г������쳣��������Ҫ�����˳� */
     /*       dsp_log(MOD_OSTIME, DSP_LOG_ERROR,
                    "[ERROR] OSTIME_SysTimeThread: The thread ID %u will exit with Calling WaitForSigleObject function! ErrorCode:%u\n",
                    gshOsTimeMMTimer, GetLastError());*/
            break;
        }
    }

    /* �ͷŶ�ʱ���ص�����(OSTIME_SysTimeCallback)�Ļ��������ڴ� */
    if (NULL != gspstTimersArray)
    {
        HeapFree(gshOSTimeHeap, 0, gspstTimersArray);
        gspstTimersArray = NULL;
    }

    /* �������ڴ����ʱ��Ҫ�����ȱ��Ϊ������� */
    gslSizeLpTimers = 0;

    return 0;
}

/**
* ��ʱ���ڲ��߳�������������,��������ʱǰ������õĺ�������֧�ֶ��̵߳��ã�����ͬOSTIME_TimeStop����ʹ��
* @param [IN] ��
* @return OSTIME_NO_ERR ���سɹ�
*         OSTIME_ERR    ����ʧ��
*/
unsigned int OSTIME_TimeStart(VOID)
{
    /* �жϵ�ǰ�Ƿ��Ѿ������˶�ʱ���̣߳�û��������������ʱ���߳� */
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

        /* ����һ���ѹ���ʱ��ģ��ʹ�� */
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

        /* �Է��ش��󲻽����ر���ֻ�Ƕ�ʱ���̵߳����ȼ��ή��һ�� */
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
* ��ʱ���ڲ��̹߳رպ��������˳�ʱ������õĺ���.��֧�ֶ��̵߳��ã�����ͬOSTIME_TimeStart����ʹ��
* @param [IN] ��
* @return ��
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

        /* �ȴ���ʱ���߳��˳� */
        DWORD dwRetVal = WaitForSingleObject(gshOsTimeMMTimer, INFINITE);
        if ((WAIT_TIMEOUT != dwRetVal) && (WAIT_OBJECT_0 != dwRetVal))
        {
            /* ��ʱ���߳����г������쳣��������Ҫ�����˳� */
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

        /* ���Ӵ����жϣ��鿴��ʱ�����ú����ⲿ�Ƿ����ƥ�� */
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

        /*����ʱ����ID��Ҳ���и�λ*/
        gsulCurTimerID = 0;
        gslgCurTime.QuadPart = 0;
    }
}


