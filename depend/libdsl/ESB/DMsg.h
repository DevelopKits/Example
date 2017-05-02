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
* 完成日期：2014年9月30日
*
* 当前版本：2.0
*/

#pragma once

#include <libdsl/DRefObj.h>
#include <libdsl/DThreadRunner.h>
#include <libdsl/DEvent.h>
#include <libdsl/DStr.h>
#include <libdsl/GeneralMacro.h>
#include <libdsl/DHttp.h>

#include <deque>
#include <vector>
#include <string>
#include <list>
#include <map>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////
class DMsgBus;

typedef unsigned int DMHID;  // 标识MsgHandler身份的ID

enum DMSG_RESULT
{
	DMSG_RESULT_UNKNOWN					= 0,
	DMSG_RESULT_OK						= 1,
	DMSG_RESULT_FAILED					   ,
	DMSG_RESULT_FAILED_PARSER			   ,
	DMSG_RESULT_FAILED_TIMEOUT			= 500400,
};

enum DMSG_ACTION
{
	DMSG_ACTION_REQUEST					= 0,
	DMSG_ACTION_RESPONSE				= 1,
	DMSG_ACTION_ACK						= 2,
	DMSG_ACTION_UNKNOWN					= 500800,
};

#define DMSG_INVALID_ID				((unsigned int)0)	// 用户自定ID从大于0开始

#define DMSG_TYPE_UNKNOWN			0		// 不应该出现的类型
#define DMSG_TYPE_DST_NOT_FOUND		1		// 接收者不存在，系统退回给发送者
#define DMSG_TYPE_DST_NOT_ACCEPT 	2		// 接收者不处理此消息，退回给发送者
#define DMSG_TYPE_USER_BEGIN 		1000	// 用户自定义消息应从1000开始

#define DMSG_FLAG_MSG_NORMAL		0		// 普通消息，放在队列尾，按先后顺序处理
#define DMSG_FLAG_MSG_URGENCY		1		// 紧急消息，放在队列头，尽快处理

// 自定义 rtti 功能
#define DECLARE_RTTI_CLASS(className) \
	DECLARE_RTTI_CLASS_CLASSNAME(className) \
	DECLARE_RTTI_CLASS_CLASSTYPE(className)


#define DECLARE_RTTI_CLASS_NO_DESTRUCTOR(className) \
	virtual ~className(){} \
	DECLARE_RTTI_CLASS_CLASSNAME(className) \
	DECLARE_RTTI_CLASS_CLASSTYPE(className)


#define DECLARE_RTTI_CLASS_CLASSNAME(className) \
	virtual const char* GetClassName(){ return #className;} \
	static const char* GetClassNameST(){return #className;}


#define DECLARE_RTTI_CLASS_CLASSTYPE(className)	\
	virtual void* GetClassType() \
	{\
		return className::GetClassTypeST();\
	}\
	static void* GetClassTypeST()\
	{\
		static void* g_ThisClassType = NULL;\
		if(!g_ThisClassType)\
		{\
			className *pType = new className;\
			className *pTypeVt = (className*&)(*pType);\
			delete pType;\
			g_ThisClassType = pTypeVt;\
		}\
		return g_ThisClassType;\
	}

class LIBDSL_API DMsg : virtual public DRefObj
{
	/******************************************以下是2.0版本的功能*********************************************/
public:
	static void BindKernel(DMsgBus* pBindKernel);		// 用于绑定特定的bus总线。
	
	/*** 以下5个函数，返回0表示成功 ***/
	int			Request(bool bSync = false);			// 消息发送，总线路由，默认m_dst == DMSG_INVALID_ID用DMsg消息类型
	int			Response(bool bSync = false);			// 消息返回，总线路由，默认m_src == DMSG_INVALID_ID用DMsg消息类型
	int			Ack(bool bSync = false);				// 消息返回，总线路由，默认m_dst == DMSG_INVALID_ID用DMsg消息类型
														// 警告：bSync true表示同步调用，此时线程安全需要业务模块自行保证
														//		 Request Response 不能同时调用

	// 重置对象（即手动初始化）
	// 警告：如果子类重载该函数，需要手动调用基类的函数
	virtual void Reset();								

	/*** 发送数据，根据szTraderId。 ***/
	int			Send(const char* szTraderId, DHttp* pHttp);
	int			Send(const char* szTraderId, const char* szBuf, int nBufLen);

	/***
	 * 路由策略
	 * 优先级z
	 * 1. m_dst or m_src不是DMSG_INVALID_ID（对应Request Response）
	 * 2. GetMsgName()
	 *		1) 可以用SetMsgName()改变消息类型
	 *		2) 当消息类型为空时，则利用5方法路由
	 * 5. GetClassName();根据类类型路由
	***/
	const char* GetMsgName();
	void		SetMsgName(const char* szMsgName);
	DRef<DHttp> m_pReqFlMsg;							// 存储Request协议，用于兼容3.0以及之前版本
	DRef<DHttp> m_pRspFlMsg;							// 存储Response协议，用于兼容3.0以及之前版本
	std::string m_TraderId;								// 区分不同的Trader
	
	/**********************************************************************************************************/
	/***
	 * 简单方法调用接口
	 * 参数：
	 *		bSync		是否同步调用, true表示同步.
	 *		szMsgName	消息类型(用于消息路由)
	 *		jsonParam	通用参数
	 * 返回值：仅表示系统调用是否成功。警告：不用于具体业务返回值。
	***/
	static int SimpleCall(bool bSync, const char* szMsgName, Json::Value &jsonInParam, DMHID src = DMSG_INVALID_ID, DMHID dst = DMSG_INVALID_ID);
	static int SimpleCall(const char* szMsgName, Json::Value &jsonInParam, Json::Value &jsonOutParam,
								 DMHID src = DMSG_INVALID_ID, DMHID dst = DMSG_INVALID_ID);
	static int SimpleCall(const char* szMsgName, void* pParam1 = NULL, void* pParam2 = NULL, void* pParam3 = NULL, 
								 void* pParam4 = NULL, void* pParam5 = NULL, void* pRetParam = NULL, 
								 DMHID src = DMSG_INVALID_ID, DMHID dst = DMSG_INVALID_ID);

	Json::Value m_jsonInValue;							// 通用参数载体（json）
	Json::Value m_jsonOutValue;							// 通用参数载体（json）

	void*		m_pParam1;								// 通用参数载体（用户自定义使用）
	void*		m_pParam2;								// 通用参数载体（用户自定义使用）
	void* 		m_pParam3;								// 通用参数载体（用户自定义使用）
	void* 		m_pParam4;								// 通用参数载体（用户自定义使用）
	void* 		m_pParam5;								// 通用参数载体（用户自定义使用）
	void*		m_pRetParam;							// 通用参数载体（用户返回值）
	/**********************************************************************************************************/ 

	void		SetFlag(int nFlag = DMSG_FLAG_MSG_NORMAL){ m_nFlag = nFlag; };
	DMSG_ACTION GetAction() { return m_actType; }
	uint32_t	MsgSeq(){ return m_nMsgSeq; }			// 消息唯一序列
	const char* Trace();								// 错误跟踪
	void		SetResult(DMSG_RESULT nResult){ m_nResult = nResult; }
	DMSG_RESULT GetResult(){ return m_nResult; }		// 返回错误码
	void		SetStartupTime(uint32_t nStartupTim = 0); // 设置消息开始时间，用于 超时处理
	void		SetTimeout(uint32_t nTimeout){ m_nTimeout = nTimeout; };
	int64_t		GetTimeout(){ return m_nStartupTime + m_nTimeout; }
	uint32_t	CheckTimeout();							// 用于超时检测

	DECLARE_RTTI_CLASS(DMsg);							// 动态类型检测

protected:
	int			PushMsg(bool bSync);
	virtual const char* CustomizedTrace() { return ""; }
	std::string m_sCustomized;
	std::string m_sTrace;

private:
	DMSG_ACTION		m_actType;
	uint32_t		m_nStartupTime;
	uint32_t		m_nTimeout;							// The minimum unit for 5 seconds
	uint32_t		m_nMsgSeq;							// 消息全局唯一序列号
	int				m_nFlag;							// 用于指明消息紧迫度

	static DAtomic	g_nMsgSeq;
	DMSG_RESULT		m_nResult;							// 0 indicate: Unknown
	static DMsgBus* g_pBindMsgBus;						// 用于绑定特点 内核 所用。

	std::string		m_sMsgName;							// 用作类型判定，默认为 className
	/**********************************************************************************************************/


	/******************************************以下是1.0版本的功能*********************************************/
public:
	DMsg( int type = DMSG_TYPE_UNKNOWN, DMHID src = DMSG_INVALID_ID, DMHID dst = DMSG_INVALID_ID );
	virtual ~DMsg() {}
	inline void SetType( int type ) { m_type = type; return; }
	inline int  GetType() const { return m_type; }

	inline void SetID( DMHID src, DMHID dst ) { m_src = src; m_dst = dst; return; }
	inline void SetSrcID( DMHID src ) { m_src = src; return; }
	inline void SetDstID( DMHID dst ) { m_dst = dst; return; }
	inline DMHID GetSrcID() const { return m_src; }
	inline DMHID GetDstID() const { return m_dst; }

protected:
	int   m_type;  // 消息类型 0 ~ 999 是系统保留值
	DMHID m_src;
	DMHID m_dst;
	/**********************************************************************************************************/
};



/**************************************以下宏用于MSG的内容打印和跟踪***************************************/
#define HANDLE_MSG_TIMEOUT						20 * 1000	// Unit: milliseconds

#define CUSTOMIZED_TRACE_BUF					8 * 1024
#define BEGIN_MSG_CUSTOMIZED_TRACE()			virtual const char* CustomizedTrace() \
{ char sBuf[CUSTOMIZED_TRACE_BUF + 1]; m_sCustomized = ""; sBuf;

#define ON_TRACE_STRING(entry)		sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%s]  ", #entry, entry.c_str());\
	m_sCustomized += sBuf;
#define ON_TRACE_CHAR(entry)		sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%s]  ", #entry, entry);\
	m_sCustomized += sBuf;
#define ON_TRACE_CHARACTER(entry)	sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%c]  ", #entry, entry);\
	m_sCustomized += sBuf;
#define ON_TRACE_INT(entry)			sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%d]  ", #entry, entry);\
	m_sCustomized += sBuf;
#define ON_TRACE_UINT(entry)		sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%u]  ", #entry, entry);\
	m_sCustomized += sBuf;
#define ON_TRACE_CMD(cmd)			if(cmd.GetPointer()){ sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%s]  ", #cmd, cmd->Trace());\
	m_sCustomized += sBuf; }
#define ON_TRACE_SIP(sip)			if(sip.GetPointer()){ sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%s]  ", #sip, sip->getBody());\
	m_sCustomized += sBuf; }
#define ON_TRACE_DOUBLE(entry)		sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%f]  ", #entry, entry);\
	m_sCustomized += sBuf;
#define ON_TRACE_POINT(entry)		sprintf_s(sBuf, CUSTOMIZED_TRACE_BUF, "%s[%p]  ", #entry, entry);\
	m_sCustomized += sBuf;

#define END_MSG_CUSTOMIZED_TRACE()				return m_sCustomized.c_str();}


// 用于 继承打印内容
#define BEGIN_MSG_CUSTOMIZED_TRACE_INHERITED_BASECLASS(baseClass)	\
	virtual const char* CustomizedTrace(){ m_sCustomized = baseClass::CustomizedTrace(); \
	m_sCustomized += "  ";	char sBuf[CUSTOMIZED_TRACE_BUF + 1];
/**********************************************************************************************************/



// 退回给用户的消息，里面存放的是原始消息
class LIBDSL_API DMsgWrap : public DMsg
{
public:
	DMsgWrap( const DRef<DMsg> & msg, int type = DMSG_TYPE_UNKNOWN, DMHID src = DMSG_INVALID_ID, DMHID dst = DMSG_INVALID_ID ) 
		: DMsg( type, src, dst ), m_msg( msg ) {}

	inline const DRef< DMsg > & GetMsg() const { return m_msg; }
	inline void SetMsg( const DRef<DMsg> & msg ) { m_msg = msg; return; }

protected:
	DRef<DMsg> m_msg;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL