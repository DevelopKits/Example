#ifndef __MS_TIMER
#define __MS_TIMER

/* C++��׼��ͷ�ļ� */
#include <list>
#include <map>
#include <windows.h>

/* ��ʱ���Ķ�ʱ���ȷ�Χ */
#define OSSYSTIME_MININTERVAL (1)
#define OSSYSTIME_MAXINTERVAL (65535)
#define IMOS_UNUSED_ARG(x)    ((VOID) x)

/* ��ʱ������������ */
#define OSTOTAL_TIMERID_TRYNUM (1000)

/* ��ʱ��Flag��� */
#define  OSTIME_KILL_SYNCHRONOUS  0x1
#define  OSTIME_PERIODIC 0x2
#define  OSTIME_ADJUST_PERIODIC   0x4    //�û��ڻص��������ܹ���̬�޸Ķ�ʱ���´δ�����ʱ����

/* ���嶨ʱ�������Ĵ����� */
#define  OSTIME_NO_ERR  0
#define  OSTIME_ERR     1

#define  OSTIME_PREC                 1000     //��ǰ��ʱ���ľ���ϸ��
#define  OSTIME_MAX_DELAY_CYC        10       //����ʱ����ʱ����10������ʱ����Ҫ���ж�ʱ��ʱ�临λ
#define  OSTIME_33MS_ADJUST_TIME_VAL 40       //������Ϊ33ms�Ľ��������޸�ֵ�����ڹ��IPC 30֡ÿ��ʱ֡�ʲ�������

/**
* ��ʱ���ص��������Ͷ���
* @param [IN] ulTimerID ִ�лص������Ķ�ʱ��ID��
* @param [IN] ulmsg ����δʹ��
* @param [IN] ulUser ��ʱ���ص�����Я���Ĳ�����Ϣ���ò�����OstimeSetEvent������ulUser��������
* @param [IN] ul1 ����δʹ��
* @param [IN] ul2 ����δʹ��
* @return ��
*/
typedef VOID (* LPOSTIMECALLBACK)(IN unsigned int ulTimerID, IN unsigned int ulmsg, IN ULONG ulUser, IN unsigned int ul1, IN unsigned int ul2);

/* ��ʱ���е�list�����еĿ��ƿ�ṹ */
typedef struct tagOS_TIMERENTRY
{
	unsigned int             ulDelay;  /* �ȴ���ʱ�� */
	unsigned int             ulResol;
	LPOSTIMECALLBACK  lpFunc; /* �ö�ʱ���Ļص����� */
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
	/* ��ʱ������ */
	HANDLE                 m_hOsTimeMMTimer ;   /*��ʱ���߳̾��*/
	PSTOSTIMERENTRY        m_pstOsTimersList ;  /*��ʱ�����ƶ���*/
	PSTOSTIMERENTRY        m_pstTimersArray ;   /*���ڱ���ص�����������*/
	int                    m_lSizeLpTimers;     /*��ʱ���ص�����������ڴ泤��*/
	HANDLE                 m_hOsTimeKillEvent ; /*��ʱ��ͬ���˳����*/
    HANDLE                 m_hOsTimeWakeEvent ; /*��ʱ���̴߳����¼�*/
	BOOL                   m_bOsTimeToDie;      /*��ʱ���߳��˳����*/
	HANDLE                 m_hOSTimeHeap;       /*���ڶ�ʱ��ģ���ڴ�����*/
	/* ��TimerID�����йصı��� */
	std::map<unsigned int, unsigned int> m_mapOsTimerID;              /*��ʱ��IDά������*/
    CRITICAL_SECTION       m_OSTimerID_cs;              /*��ʱ��IDά���ؼ���*/
    CRITICAL_SECTION       m_GetSystemTime_cs;          /*��ʱ�����ʱ���ȡ�ؼ���*/
    CRITICAL_SECTION       m_OsTime_cs;                 /*��ʱ�����ƿ�ά���ؼ���*/
    LARGE_INTEGER          m_lgCurTime;                 /*ϵͳ����������ʱ��*/
    unsigned int           m_ulCurTimerID ;          /*��һ��ȫ�ֵĶ�ʱ��ID�� */

	unsigned long          m_ulTimerID; //��ʱ��ID

};
#endif