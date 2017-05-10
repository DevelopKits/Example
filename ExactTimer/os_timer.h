
/* ʵ�ֶ�ʱ����ʵ�ֺ��� */
#ifndef _XP_OS_TIMER_H_
#define _XP_OS_TIMER_H_

/* ��ʱ������������ */
#define OSTOTAL_TIMERID_TRYNUM (1000)
#define IN 
#define OUT
#define INOUT

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
    struct tagOS_TIMERENTRY*  pstNextOsTimerEntry;
} STOSTIMERENTRY, *PSTOSTIMERENTRY;


/* ��ʱ������⺯���ӿ� */

/**
* ��ʱ���ڲ��߳�������������,��������ʱǰ������õĺ�������֧�ֶ��̵߳��ã�����ͬOSTIME_TimeStop����ʹ��
* @param [IN] ��
* @return OSTIME_NO_ERR ���سɹ�
*         OSTIME_ERR    ����ʧ��
*/
unsigned int OSTIME_TimeStart(VOID);

/**
* ��ʱ���ڲ��̹߳رպ��������˳�ʱ������õĺ���.��֧�ֶ��̵߳��ã�����ͬOSTIME_TimeStart����ʹ��
* @param [IN] ��
* @return ��
*/
VOID OSTIME_TimeStop(VOID);

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
unsigned int OstimeSetEvent(IN unsigned int ulDelay, IN unsigned int ulResol, IN LPOSTIMECALLBACK lpFunc, IN ULONG ulUser, IN unsigned int ulFlags);

/**
* ֹͣһ����ʱ��
* @param [IN] ulTimerID ��ʱ��ID�š�
* @return OSTIME_ERR ��ʾɾ����ʱ��ʧ��
*      OSTIME_NO_ERR ��ʾɾ����ʱ���ɹ�
*/
unsigned int OstimeKillEvent(IN unsigned int ulTimerID);

#endif

