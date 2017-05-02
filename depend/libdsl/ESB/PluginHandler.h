/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* PluginHandler.h : �������ģ��
* ����    ��������
* ������ڣ�2014��9��19��
*
* ��ǰ�汾��1.0
*/

#pragma once

#include <libdsl/ESB/DMsgHandler.h>


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////


/***
 * ����ӿں�������Init,OnBusRunning�ȼ�Ϊ�ӿ����֡�  
 * ����ֵ��0��ʾ�ɹ�������ֵ��ʾ����
 * pMsgBus����Ҫ�󶨵������bus
 * pBusParam��Ԥ�����ݲ�ʹ��
***/
extern "C" typedef int (*Init)(DMsgBus* pMsgBus, void* pBusParam);				// ��ʼ�����
extern "C" typedef int (*OnBusRunning)(DMsgBus* pMsgBus, void* pBusParam);		// ֪ͨ�����������������
extern "C" typedef int (*OnBeforeAnti)(DMsgBus* pMsgBus, void* pBusParam);		// ֪ͨ���, ׼������������������Դ��
extern "C" typedef int (*Anti)(DMsgBus* pMsgBus, void* pBusParam);				// ����ʼ����������Դ��

class LIBDSL_API Plugin : virtual public DRefObj
{
public:
	Plugin() { m_hDll = NULL; m_bInit = false; }
	virtual ~Plugin() { if(m_hDll != NULL) dlclose(m_hDll); }

	int Load();
	int InitPlugin(DMsgBus* pMsgBus, void* pBusParam);
	int OnBusRunningPlugin(DMsgBus* pMsgBus, void* pBusParam);
	int OnBeforeAntiPlugin(DMsgBus* pMsgBus, void* pBusParam);
	int AntiPlugin(DMsgBus* pMsgBus, void* pBusParam);
	bool IsInit(){ return m_bInit; }

	std::string		m_sName;
	std::string		m_sPath;

private:
	bool			m_bInit;
	HDDLL			m_hDll;
	
	Init			m_funcInit;
	OnBusRunning	m_funcOnBusRunning;
	OnBeforeAnti	m_funcOnBeforeAnti;
	Anti			m_funcAnti;
};

class LIBDSL_API PluginHandler : public DMsgHandler
{
public:
	PluginHandler(){ SetThreadNum(0); }		// ʹ��bus�Ĺ����߳�
	virtual ~PluginHandler(){}

	DECLARE_FUNC_MAP(PluginHandler)

public:
	// szFile�� exe�����·������ /bin/dlls/�� 
	//		    �������Ŀ¼�µ�����.dll��windows������.so��linux���Ľ�β�ļ���
	//		    ��������Ŀ¼��
	// ·����ʽ��/ + ·�� + /, ��/bin/dlls/
	// ����ֵ�� �Ѿ������·���������󷵻�С��0
	int AddPluginPath(const char* szPath);

protected:
	virtual int OnInitHandler();
	virtual int OnBusRunning();
	virtual int OnBeforeAntinitHandler();
	virtual int OnAntinitHandler();

private:
	int LoadAllPlugin();
	int LoadPlugin(const char* szPath);
	void AddPlugin( const char* szName, const char* szPath );

	int InitAllPlugin();
	int OnBusRunningAllPlugin();
	int OnBeforeAntiAllPlugin();
	int AntiAllPlugin();

private:
	std::vector<std::string> m_vecPluginPath;
	std::vector< DRef<Plugin> > m_vecPlugin;
};



/////////////////////////////////////////////////
END_NAMESPACE_DSL