/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* DMsgBus.h : 消息总线对象
* 作者　  ：周鉴刚
*
* 版本    ：2.0
* 修订记录：新增功能：bus寻址和路由；msg自产ID，错误跟踪，bus动态绑定，消息发送等
* 作者    ：吕少卿
* 完成日期：2014年9月17日
*
* 当前版本：2.0
*/

#ifndef	INCLUDED_LIBDSL_DMSGBUS_H
#define	INCLUDED_LIBDSL_DMSGBUS_H

#include <libdsl/DNetEngine.h>
#include <libdsl/ESB/DMsg.h>
#include <libdsl/ESB/DMsgHandler.h>
#include <libdsl/ESB/ESBProtocal.h>
#include <libdsl/ESB/DTrader.h>
#include <libdsl/ESB/ISession.h>


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////


// NOTE ：在OnMsg()的实现中，往往需要msg转换成具体的子类，可以使用DRef::Cast()函数来实现
// 例如 DRef<DMsg> msg = new DMsgWrap(); DRef< DMsgWrap > msgwrap = msg.Cast<DMsgWrap>();


#define KERNEL_TIME_WAIT			5 * 1000

class LIBDSL_API DMsgBus
{
	/******************************************以下是2.0版本的功能*********************************************/
public:
	DMsgBus( unsigned int nThreadNum = 1, unsigned int nMaximumQueue = 800 * 1024 );
	virtual ~DMsgBus();
	static DMsgBus*	MsgBus();

	int InitKernel();
	int AntinitKernel();

	/***
	 * 警告：内核启动后，不能增加服务，也不能删除服务。 服务生命周期由内核管理。
	 * bPersistence： 表示服务需要持久化，会最后才删除（AntinitKernel中不会删除）。 一般都是 false。
	 * Kernel manage the Handler and Handler life cycle
	***/
	int AddHandler(DRef<DMsgHandler> pHandler, bool bPersistence = false);
	int DispatchCmd(DRef<DMsg> pCmd);
	unsigned int SetTimer(unsigned int nTimeMs, DTimerHandler* pHandler);
	unsigned int KillTimer(unsigned int nTimerId);
	
	/*** 交易员: 发送数据，根据szTraderId。 ***/
	int	Send(const char* szTraderId, DHttp* pHttp);
	int	Send(const char* szTraderId, const char* szBuf, int nBufLen);

	int IntoKernelConsole();						// Into the kernel console, and the thread will switch to the console.
	void AddWndPrgVersionInfo(const char* prgVer);	// 在程序窗口最前面加上版本信息

	// 只有设置SetNetEngine的线程数后，网络引擎才启动。 默认线程数是0。
	int			StartNetEngine(int nThreadNum);
	DNetEngine*	GetNetEngine() { return &m_oNetEngine; }

protected:
	/***
	 * 警告：子类重载这些函数后，必须在最后手动调用父类的函数，否则部分功能不可用。
	 * InitKernel() 中，会调用以下3个函数
	***/
	virtual int OnInitKernel();
	virtual int BeforeKernelRunning();
	virtual int OnBusRunning();

	virtual int OnKernelConsole();

private:
	std::list<DMsgHandler*>* FindHandler(DMsg* pHandlerCmd);
	std::list<DMsgHandler*>* FindHandler(DMsg* pHandlerCmd, std::map< std::string, std::list<DMsgHandler*> > &mapHandlerFunc);

	int BindHandlerFuncs(DMsgHandler* pHandler);
	int BindHandlerFuncsInside(std::map< std::string, std::list<DMsgHandler*> >& mapHandlerFuncs, DMsgHandler* pHandler, 
							   std::list<std::string>::iterator itBegin, std::list<std::string>::iterator itEnd);

	uint32_t ProcessMsg(DMsgHandler* pHandler, const DRef< DMsg > & msg, int flag, bool bSnyc);
	uint32_t DispatchMsg();
	uint32_t DispatchTimer();
	int		 InvokeHandlerMsg(DMsg* pMsg);
	uint32_t TimeoutHandler();
	void	 ResetDispatchTimeout();
	bool	 IsDispatchTimeout();
	
private:
	std::vector< DRef<DMsgHandler> >	m_vecHandlers;
	std::list< DRef<DMsgHandler> >		m_listPersistenceHandlers;	 
	dsl::DMutex							m_mtxHandler;

	unsigned int						m_maximumQueue;
	DEvent								m_evtMsgs;
	std::deque< DRef<DMsgHandler> >		m_deqHandlerMsg;
	std::map< std::string, std::list<DMsgHandler*> >	m_mapHandlerRequestFuncs;
	std::map< std::string, std::list<DMsgHandler*> >	m_mapHandlerResponseFuncs;

	class TTimer
	{
	public:
		unsigned int m_nId;
		DRef<DTimerHandler> m_pHandler;
		uint32_t m_n64NextTick;
		int m_nTimeoutMs;
	};
	void InsertTimer( TTimer* pTimer );
	
	DMutex					m_mtxTimers;
	std::list< TTimer >		m_listTimers;
	unsigned int			m_nDispachTimeout;

	DNetEngine				m_oNetEngine;
	int						m_nNetThreadNum;

	/********************************************************************/
	/***
	 * 交易员。
	***/
	friend class DTrader;
	void AddTrader(DRef<DTrader> pTrader);
	void DelTrader(DRef<DTrader> pTrader);
	ISession<std::string, DRef<DTrader> >	m_senTradersString;
	/********************************************************************/

	static DMsgBus*			g_appMsgBus;
	/**********************************************************************************************************/

public:

	/******************************************以下是1.0版本的功能*********************************************/
	int Start();
	int SignalStop();
	int Stop();

	// NOTE : 添加的handler数不可超过65535个
 	// int DelHandler( const DRef<DMsgHandler> & handler ); // 暂时不提供删除功能

	DMHID FindHandler( const char * name );

	/***
	 * 用于调度策略，根据msg的名字
	 * 调度策略优先级
	 * 1. m_dst or m_src不是DMSG_INVALID_ID（对应Request Response）
	 * 2. GetMsgName()
	 *		1) CFLMessage存在，则根据szFlMsgName返回该名字
	 *		2) CFLMessage不存在，则返回DMsg的类名（GetClassName()）
	***/
	int PushMsg( const DRef< DMsg > & msg, int flag = 0, bool bSnyc = false );

	int QueueSize( void );
	bool IsRunning( void ) const { return m_isRunning; }
	void SetThreadName(const char * name); // 设置线程名称 最长15字节 必须在Start前调用

protected:
	int driver_engine( DThread * th, int index );
	static int driver_func( void * arg, void * th );

protected:
	class  PerThreadInfo
	{
	public:
		int id;
		DMsgBus* pMsgBus;
	};

	unsigned int m_threadNum;

	std::vector< PerThreadInfo > m_vecArgInfo;
	std::vector< DRunner<void> > m_vecThread;
	bool m_isRunning;
	/**********************************************************************************************************/
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DMSGBUS_H

