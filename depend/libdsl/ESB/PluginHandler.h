/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* PluginHandler.h : 插件管理模块
* 作者    ：吕少卿
* 完成日期：2014年9月19日
*
* 当前版本：1.0
*/

#pragma once

#include <libdsl/ESB/DMsgHandler.h>


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////


/***
 * 插件接口函数，如Init,OnBusRunning等即为接口名字。  
 * 返回值：0表示成功，其他值表示错误
 * pMsgBus：需要绑定到的相关bus
 * pBusParam：预留，暂不使用
***/
extern "C" typedef int (*Init)(DMsgBus* pMsgBus, void* pBusParam);				// 初始化插件
extern "C" typedef int (*OnBusRunning)(DMsgBus* pMsgBus, void* pBusParam);		// 通知插件，可以正常运行
extern "C" typedef int (*OnBeforeAnti)(DMsgBus* pMsgBus, void* pBusParam);		// 通知插件, 准备清理工作（不清理资源）
extern "C" typedef int (*Anti)(DMsgBus* pMsgBus, void* pBusParam);				// 反初始化（清理资源）

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
	PluginHandler(){ SetThreadNum(0); }		// 使用bus的共享线程
	virtual ~PluginHandler(){}

	DECLARE_FUNC_MAP(PluginHandler)

public:
	// szFile： exe的相对路径，如 /bin/dlls/。 
	//		    搜索这个目录下的所有.dll（windows）或者.so（linux）的结尾文件。
	//		    不包括子目录。
	// 路径格式：/ + 路径 + /, 如/bin/dlls/
	// 返回值： 已经加入的路径数，错误返回小于0
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