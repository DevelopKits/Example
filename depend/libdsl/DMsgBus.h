/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* DMsgBus.h : ��Ϣ���߶���
* ���ߡ�  ���ܼ���
*
* �汾    ��2.0
* �޶���¼���������ܣ�busѰַ��·�ɣ�msg�Բ�ID��������٣�bus��̬�󶨣���Ϣ���͵�
* ����    ��������
* ������ڣ�2014��9��17��
*
* ��ǰ�汾��2.0
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


// NOTE ����OnMsg()��ʵ���У�������Ҫmsgת���ɾ�������࣬����ʹ��DRef::Cast()������ʵ��
// ���� DRef<DMsg> msg = new DMsgWrap(); DRef< DMsgWrap > msgwrap = msg.Cast<DMsgWrap>();


#define KERNEL_TIME_WAIT			5 * 1000

class LIBDSL_API DMsgBus
{
	/******************************************������2.0�汾�Ĺ���*********************************************/
public:
	DMsgBus( unsigned int nThreadNum = 1, unsigned int nMaximumQueue = 800 * 1024 );
	virtual ~DMsgBus();
	static DMsgBus*	MsgBus();

	int InitKernel();
	int AntinitKernel();

	/***
	 * ���棺�ں������󣬲������ӷ���Ҳ����ɾ������ ���������������ں˹���
	 * bPersistence�� ��ʾ������Ҫ�־û���������ɾ����AntinitKernel�в���ɾ������ һ�㶼�� false��
	 * Kernel manage the Handler and Handler life cycle
	***/
	int AddHandler(DRef<DMsgHandler> pHandler, bool bPersistence = false);
	int DispatchCmd(DRef<DMsg> pCmd);
	unsigned int SetTimer(unsigned int nTimeMs, DTimerHandler* pHandler);
	unsigned int KillTimer(unsigned int nTimerId);
	
	/*** ����Ա: �������ݣ�����szTraderId�� ***/
	int	Send(const char* szTraderId, DHttp* pHttp);
	int	Send(const char* szTraderId, const char* szBuf, int nBufLen);

	int IntoKernelConsole();						// Into the kernel console, and the thread will switch to the console.
	void AddWndPrgVersionInfo(const char* prgVer);	// �ڳ��򴰿���ǰ����ϰ汾��Ϣ

	// ֻ������SetNetEngine���߳�������������������� Ĭ���߳�����0��
	int			StartNetEngine(int nThreadNum);
	DNetEngine*	GetNetEngine() { return &m_oNetEngine; }

protected:
	/***
	 * ���棺����������Щ�����󣬱���������ֶ����ø���ĺ��������򲿷ֹ��ܲ����á�
	 * InitKernel() �У����������3������
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
	 * ����Ա��
	***/
	friend class DTrader;
	void AddTrader(DRef<DTrader> pTrader);
	void DelTrader(DRef<DTrader> pTrader);
	ISession<std::string, DRef<DTrader> >	m_senTradersString;
	/********************************************************************/

	static DMsgBus*			g_appMsgBus;
	/**********************************************************************************************************/

public:

	/******************************************������1.0�汾�Ĺ���*********************************************/
	int Start();
	int SignalStop();
	int Stop();

	// NOTE : ��ӵ�handler�����ɳ���65535��
 	// int DelHandler( const DRef<DMsgHandler> & handler ); // ��ʱ���ṩɾ������

	DMHID FindHandler( const char * name );

	/***
	 * ���ڵ��Ȳ��ԣ�����msg������
	 * ���Ȳ������ȼ�
	 * 1. m_dst or m_src����DMSG_INVALID_ID����ӦRequest Response��
	 * 2. GetMsgName()
	 *		1) CFLMessage���ڣ������szFlMsgName���ظ�����
	 *		2) CFLMessage�����ڣ��򷵻�DMsg��������GetClassName()��
	***/
	int PushMsg( const DRef< DMsg > & msg, int flag = 0, bool bSnyc = false );

	int QueueSize( void );
	bool IsRunning( void ) const { return m_isRunning; }
	void SetThreadName(const char * name); // �����߳����� �15�ֽ� ������Startǰ����

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

