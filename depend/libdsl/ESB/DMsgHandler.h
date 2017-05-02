/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* DMsgHandler.h : ��Ϣ����ģ��
* ���ߡ�  ���ܼ���
*
* �汾    ��2.0
* �޶���¼���������ܣ�Ѱַ��·�ɣ��̵߳��ȣ���ʱ����
* ����    ��������
* ������ڣ�2014��9��18��
*
* ��ǰ�汾��2.0
*/

#pragma once

#include <libdsl/DTimerMgr.h>
#include <libdsl/ESB/DMsg.h>
#include <libdsl/ESB/IProfiles.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////
class DMsgBus;
class IProfiles;

#define HANDLER_THREAD_MAX_NUMBER			32
#define MAX_VALUE_QUEUE						100000

#define THREADS_SHARING_MODE				1	// �����߳�ģʽ
#define THREADS_EXCLUSIVE_MODE				2	// �����߳�ģʽ

// ��Ϣ�������Ļ��࣬�û��̳д���ʵ�־������Ϣ����
// ϵͳ�ᱣ֤ͬһ��DMsgHandler�յ�����Ϣ�Ǵ��л����лص�
class LIBDSL_API DMsgHandler : virtual public DTimerHandler
{
/******************************************������2.0�汾�Ĺ���*********************************************/
public:
	DMsgHandler();							// ���棺OnBusRunning �����ٸı��߳���
	int SetThreadNum(int nThreadNum);		// ����ģ�������߳���
											// Ĭ��0, ��ʾʹ��bus�Ĺ����̣߳�����������߳�
	int GetThreadNum();
	void AddProfiles(IProfiles* pProfiles); // ���������ļ�����ģ�顣ÿ��handlerֻ�ܴ���һ����

	/* ���º�����ֻ����OnInitHandler()�л���֮�󣬲���ʹ�á�(���캯���в���ʹ��) */
	unsigned int			SetTimer(unsigned int nTimeMs); 
	int			KillTimer(unsigned int nTimerId);
	DMsgBus*	GetMsgBus();
	const char* GetHandlerName() { return GetClassName(); }

protected:
	/*****************************************************************************/
	/***
	 * ���º�����������̳�ʹ��
	 * ���棺���ϸ�����������˵����ʹ����Щ����
	***/
	// ���ڳ�ʼ������������ں����е��ñ������һЩ��ʼ��������ʱ���ɵ������� Handler �ࣩ
	virtual int OnInitHandler(); 
	// ���ڷ���ʼ��������պ��෴����������ڸú����д���Handler�˳���������ʱ���ɵ������� Handler �ࣩ
	virtual int OnAntinitHandler(); 
	// ��������ʼ������������ڸú����д��� Handler �˳�ǰ�Ĺ�������ʱ�ɵ������� Handler �ࣩ
	// ���棺����Handler��ʱ���ܻ�Ҫʹ�ø�Handler�����ܹرո�Handler���ͷ���Դ��
	virtual int OnBeforeAntinitHandler(); 
	// �����������������ʹ������һ����Դ������ʱ�ɵ������� Handler �ࣩ
	virtual int OnBusRunning(); 
	/*****************************************************************************/
	
	virtual void OnTimer( unsigned int nTimerId );

	/*****************************************************************************/
	/***		   ��Ϣ������Ϣ�ַ�����Ϣ��ʱ��⣬��Ϣɾ���ȴ��� 		   ***/
	int		ProcessMsg(dsl::DRef<DMsg> pObj, int nFlag, bool bSnyc);
	void	AddDelayProcessMsg( DMsg* pMsg );
	void	DelDelayProcessMsg( DMsg* pMsg );
	void	DelDelayProcessMsg( unsigned int nMsgSeq );
	virtual int	OnRunner(dsl::DThread * th, int nThreadMode);
	uint32_t	CheckMsgTimeout();
	void		DispatchTimer();
	/*****************************************************************************/

protected:
	std::list< DRef<DMsg> >		m_listMsgsTimeout;
	std::list<std::string>		m_listHandlerRequestFuns;
	std::list<std::string>		m_listHandlerResponseFuns;
	DRef<IProfiles>				m_pProfiles;

private:
	DMsgBus*					m_pMsgBus;
	dsl::DEvent					m_evtRunner;  // ���� OnRunner ����
	std::list< DRef<DMsg> >		m_listRunnerMsgs;
	std::list<unsigned int>		m_listTimers; // ���ڶ�ʱ��
	DMutex						m_mtxRunner;
	DRunner<DMsgHandler>		*m_pRunner;
	int							m_nThreadNum; // MSG_THREAD_MAX_NUMBER
	bool						m_bRunner;	  // ���� OnRunner ����
	DMutex						m_mtxMsgs4Timeout;
	
private:
	// for DMsgBus
	/*****************************************************************************/
	/***
	 * th: thread info
	 * return: The return value is the thread exit code
	 * nRunnerNum: the number of threads
	 * ���棺OnRunner() ���Ƕ��̲߳������⣡
	 * ���棺�ص��������߳�ΪStartRunner()�������ģ����ں��߼��̣߳�
	***/
	int		StartRunner(); 
	int		StopRunner(bool bWaitRunnerExit = true);
	void	WaitRunnerExit();
	int		CallRunner( dsl::DThread * th );
	bool	IsRunning();
	void	WaitRunning(unsigned int nTimerMs);
	void	SetEvent4Running();

	int		KernelRunning() { return OnBusRunning(); }
	int		BeforeAntinitHandler() { return OnBeforeAntinitHandler(); }
	void	BindToKernel(DMsgBus* pMsgBus);
	virtual void OnTimeout( unsigned int nTimerId );

	std::list<std::string>::iterator BeginRequestFuns() { return m_listHandlerRequestFuns.begin(); }
	std::list<std::string>::iterator BeginResponseFuns() { return m_listHandlerResponseFuns.begin(); }
	std::list<std::string>::iterator EndRequestFuns() { return m_listHandlerRequestFuns.end(); }
	std::list<std::string>::iterator EndResponseFuns() { return m_listHandlerResponseFuns.end(); }
	/*****************************************************************************/

protected:
	virtual int InitHandler();
	int			AntinitHandler();
	virtual int Invoke(DMsg* pMsg);
	virtual int Invoke4Timeout(DMsg* pMsg);

	DECLARE_RTTI_CLASS(DMsgHandler);
/**********************************************************************************************************/


public:
/******************************************������1.0�汾�Ĺ���*********************************************/
	inline DMHID GetID() { return m_id; }
	inline DMHID SetID(DMHID id) { m_id = id; return m_id; }
	virtual int OnMsg( const DRef<DMsg> & msg );

protected:
	virtual ~DMsgHandler() {}

private:
	friend class DMsgBus;
	DMHID m_id; // ����ʱ���ã��˳�ʱ�������DMsgBus����ά��
/**********************************************************************************************************/
};

#define THIS_PROFILES(className)	((className*)m_pProfiles.GetPointer())


#define DECLARE_FUNC_MAP(className) \
DECLARE_RTTI_CLASS(className) \
typedef void (className::*MSG_FUNC)( dsl::DMsg* pMsg );\
struct MSG_FUNCMAP_ENTRY \
{\
	std::string m_sMsg;\
	MSG_FUNC m_pFunc;\
	MSG_FUNC m_pFuncAck;\
	MSG_FUNC m_pFuncTimeout;\
};\
typedef std::map<std::string, MSG_FUNCMAP_ENTRY> MSG_FUNCMAP;\
protected:\
	virtual int Invoke(dsl::DMsg* pMsg);\
	virtual int Invoke4Timeout(dsl::DMsg* pMsg);\
	virtual int InitHandler();\
private:\
	MSG_FUNCMAP	m_mapHandlerFuncs_Request;\
	MSG_FUNCMAP	m_mapHandlerFuncs_Response;

#define BEGIN_FUNC_MAP(className, baseClass) \
	int className::Invoke(dsl::DMsg* pMsg)\
	{\
		MSG_FUNCMAP	m_mapHandlerFuncs;\
		if(pMsg->GetAction() == dsl::DMSG_ACTION_REQUEST || pMsg->GetAction() == dsl::DMSG_ACTION_ACK)\
			m_mapHandlerFuncs = m_mapHandlerFuncs_Request;\
		else\
			m_mapHandlerFuncs = m_mapHandlerFuncs_Response;\
		MSG_FUNCMAP::iterator it = m_mapHandlerFuncs.find( pMsg->GetMsgName() );\
		if(it != m_mapHandlerFuncs.end() && it->second.m_pFunc)\
		{\
			if(pMsg->GetAction() == dsl::DMSG_ACTION_ACK)\
				(this->*(it->second.m_pFuncAck))(pMsg);\
			else\
				(this->*(it->second.m_pFunc))(pMsg);\
			return 0;\
		}\
		return baseClass::Invoke(pMsg);\
	}\
	int className::Invoke4Timeout(dsl::DMsg* pMsg)\
	{\
		MSG_FUNCMAP	m_mapHandlerFuncs;\
		if(pMsg->GetAction() == dsl::DMSG_ACTION_REQUEST || pMsg->GetAction() == dsl::DMSG_ACTION_ACK)\
			m_mapHandlerFuncs = m_mapHandlerFuncs_Request;\
		else\
			m_mapHandlerFuncs = m_mapHandlerFuncs_Response;\
		MSG_FUNCMAP::iterator it = m_mapHandlerFuncs.find( pMsg->GetMsgName() );\
		if(it != m_mapHandlerFuncs.end() && it->second.m_pFuncTimeout)\
		{\
			(this->*(it->second.m_pFuncTimeout))(pMsg);\
			return 0;\
		}\
		return baseClass::Invoke4Timeout(pMsg);\
	}\
	int className::InitHandler()\
	{\
		typedef std::pair <std::string, className::MSG_FUNCMAP_ENTRY> HandlerFuncs_Pair;\
		std::pair< std::map<std::string, className::MSG_FUNCMAP_ENTRY>::iterator, bool > pr;\
		if(baseClass::InitHandler() != 0)\
		{\
			DLOG_ERR("InitHandler failed, className[%s]", #className);\
			return -1;\
		}

#define INSERT_INTO_FUNC_MAP(msg, func, funcAck, timeoutFunc, mapFuncs, listFuncs)\
	MSG_FUNCMAP_ENTRY entry;\
	entry.m_sMsg = msg;\
	entry.m_pFunc = (MSG_FUNC)func;\
	entry.m_pFuncAck = (MSG_FUNC)funcAck;\
	entry.m_pFuncTimeout = (MSG_FUNC)timeoutFunc;\
	pr = mapFuncs.insert( HandlerFuncs_Pair(entry.m_sMsg, entry) );\
	if(!pr.second)\
	{\
		DLOG_ERR("INSERT_INTO_FUNC_MAP[%s] failed, %s", #mapFuncs, entry.m_sMsg.c_str());\
		return -1;\
	}\
	listFuncs.push_back(entry.m_sMsg);



#define ON_REQUEST_FUNC(msg, func, funcAck, timeoutFunc) { INSERT_INTO_FUNC_MAP(msg, func, funcAck, timeoutFunc, m_mapHandlerFuncs_Request, m_listHandlerRequestFuns) }
#define ON_RESPONSE_FUNC(msg, func, timeoutFunc)		 { INSERT_INTO_FUNC_MAP(msg, func, NULL, timeoutFunc, m_mapHandlerFuncs_Response, m_listHandlerResponseFuns) }



#define END_FUNC_MAP() \
	return OnInitHandler();\
}


/////////////////////////////////////////////////
END_NAMESPACE_DSL
