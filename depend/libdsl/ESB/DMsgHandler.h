/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* DMsgHandler.h : 消息处理模块
* 作者　  ：周鉴刚
*
* 版本    ：2.0
* 修订记录：新增功能：寻址和路由，线程调度，超时处理
* 作者    ：吕少卿
* 完成日期：2014年9月18日
*
* 当前版本：2.0
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

#define THREADS_SHARING_MODE				1	// 共享线程模式
#define THREADS_EXCLUSIVE_MODE				2	// 独享线程模式

// 消息处理器的基类，用户继承此类实现具体的消息处理
// 系统会保证同一个DMsgHandler收到的消息是串行化进行回调
class LIBDSL_API DMsgHandler : virtual public DTimerHandler
{
/******************************************以下是2.0版本的功能*********************************************/
public:
	DMsgHandler();							// 警告：OnBusRunning 后不能再改变线程数
	int SetThreadNum(int nThreadNum);		// 设置模块运行线程数
											// 默认0, 表示使用bus的共享线程，不需独立开线程
	int GetThreadNum();
	void AddProfiles(IProfiles* pProfiles); // 创建配置文件处理模块。每个handler只能存在一个。

	/* 以下函数，只有在OnInitHandler()中或者之后，才能使用。(构造函数中不能使用) */
	unsigned int			SetTimer(unsigned int nTimeMs); 
	int			KillTimer(unsigned int nTimerId);
	DMsgBus*	GetMsgBus();
	const char* GetHandlerName() { return GetClassName(); }

protected:
	/*****************************************************************************/
	/***
	 * 以下函数，由子类继承使用
	 * 警告：请严格按照下面文字说明，使用这些函数
	***/
	// 正在初始化，子类可以在函数中调用本身类的一些初始工作（此时不可调用其他 Handler 类）
	virtual int OnInitHandler(); 
	// 正在反初始化与上面刚好相反，子类可以在该函数中处理Handler退出工作（此时不可调用其他 Handler 类）
	virtual int OnAntinitHandler(); 
	// 即将反初始化，子类可以在该函数中处理 Handler 退出前的工作（此时可调用其他 Handler 类）
	// 警告：其他Handler此时可能还要使用该Handler，不能关闭该Handler，释放资源等
	virtual int OnBeforeAntinitHandler(); 
	// 程序启动，子类可以使用所有一切资源。（此时可调用其他 Handler 类）
	virtual int OnBusRunning(); 
	/*****************************************************************************/
	
	virtual void OnTimer( unsigned int nTimerId );

	/*****************************************************************************/
	/***		   消息处理，消息分发，消息超时检测，消息删除等待。 		   ***/
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
	dsl::DEvent					m_evtRunner;  // 用于 OnRunner 函数
	std::list< DRef<DMsg> >		m_listRunnerMsgs;
	std::list<unsigned int>		m_listTimers; // 用于定时器
	DMutex						m_mtxRunner;
	DRunner<DMsgHandler>		*m_pRunner;
	int							m_nThreadNum; // MSG_THREAD_MAX_NUMBER
	bool						m_bRunner;	  // 用于 OnRunner 函数
	DMutex						m_mtxMsgs4Timeout;
	
private:
	// for DMsgBus
	/*****************************************************************************/
	/***
	 * th: thread info
	 * return: The return value is the thread exit code
	 * nRunnerNum: the number of threads
	 * 警告：OnRunner() 考虑多线程并发问题！
	 * 警告：回调函数的线程为StartRunner()中启动的，非内核逻辑线程！
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
/******************************************以下是1.0版本的功能*********************************************/
	inline DMHID GetID() { return m_id; }
	inline DMHID SetID(DMHID id) { m_id = id; return m_id; }
	virtual int OnMsg( const DRef<DMsg> & msg );

protected:
	virtual ~DMsgHandler() {}

private:
	friend class DMsgBus;
	DMHID m_id; // 加入时设置，退出时清除，由DMsgBus负责维护
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
