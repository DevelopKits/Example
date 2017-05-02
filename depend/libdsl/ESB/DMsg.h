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
* ������ڣ�2014��9��30��
*
* ��ǰ�汾��2.0
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

typedef unsigned int DMHID;  // ��ʶMsgHandler��ݵ�ID

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

#define DMSG_INVALID_ID				((unsigned int)0)	// �û��Զ�ID�Ӵ���0��ʼ

#define DMSG_TYPE_UNKNOWN			0		// ��Ӧ�ó��ֵ�����
#define DMSG_TYPE_DST_NOT_FOUND		1		// �����߲����ڣ�ϵͳ�˻ظ�������
#define DMSG_TYPE_DST_NOT_ACCEPT 	2		// �����߲��������Ϣ���˻ظ�������
#define DMSG_TYPE_USER_BEGIN 		1000	// �û��Զ�����ϢӦ��1000��ʼ

#define DMSG_FLAG_MSG_NORMAL		0		// ��ͨ��Ϣ�����ڶ���β�����Ⱥ�˳����
#define DMSG_FLAG_MSG_URGENCY		1		// ������Ϣ�����ڶ���ͷ�����촦��

// �Զ��� rtti ����
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
	/******************************************������2.0�汾�Ĺ���*********************************************/
public:
	static void BindKernel(DMsgBus* pBindKernel);		// ���ڰ��ض���bus���ߡ�
	
	/*** ����5������������0��ʾ�ɹ� ***/
	int			Request(bool bSync = false);			// ��Ϣ���ͣ�����·�ɣ�Ĭ��m_dst == DMSG_INVALID_ID��DMsg��Ϣ����
	int			Response(bool bSync = false);			// ��Ϣ���أ�����·�ɣ�Ĭ��m_src == DMSG_INVALID_ID��DMsg��Ϣ����
	int			Ack(bool bSync = false);				// ��Ϣ���أ�����·�ɣ�Ĭ��m_dst == DMSG_INVALID_ID��DMsg��Ϣ����
														// ���棺bSync true��ʾͬ�����ã���ʱ�̰߳�ȫ��Ҫҵ��ģ�����б�֤
														//		 Request Response ����ͬʱ����

	// ���ö��󣨼��ֶ���ʼ����
	// ���棺����������ظú�������Ҫ�ֶ����û���ĺ���
	virtual void Reset();								

	/*** �������ݣ�����szTraderId�� ***/
	int			Send(const char* szTraderId, DHttp* pHttp);
	int			Send(const char* szTraderId, const char* szBuf, int nBufLen);

	/***
	 * ·�ɲ���
	 * ���ȼ�z
	 * 1. m_dst or m_src����DMSG_INVALID_ID����ӦRequest Response��
	 * 2. GetMsgName()
	 *		1) ������SetMsgName()�ı���Ϣ����
	 *		2) ����Ϣ����Ϊ��ʱ��������5����·��
	 * 5. GetClassName();����������·��
	***/
	const char* GetMsgName();
	void		SetMsgName(const char* szMsgName);
	DRef<DHttp> m_pReqFlMsg;							// �洢RequestЭ�飬���ڼ���3.0�Լ�֮ǰ�汾
	DRef<DHttp> m_pRspFlMsg;							// �洢ResponseЭ�飬���ڼ���3.0�Լ�֮ǰ�汾
	std::string m_TraderId;								// ���ֲ�ͬ��Trader
	
	/**********************************************************************************************************/
	/***
	 * �򵥷������ýӿ�
	 * ������
	 *		bSync		�Ƿ�ͬ������, true��ʾͬ��.
	 *		szMsgName	��Ϣ����(������Ϣ·��)
	 *		jsonParam	ͨ�ò���
	 * ����ֵ������ʾϵͳ�����Ƿ�ɹ������棺�����ھ���ҵ�񷵻�ֵ��
	***/
	static int SimpleCall(bool bSync, const char* szMsgName, Json::Value &jsonInParam, DMHID src = DMSG_INVALID_ID, DMHID dst = DMSG_INVALID_ID);
	static int SimpleCall(const char* szMsgName, Json::Value &jsonInParam, Json::Value &jsonOutParam,
								 DMHID src = DMSG_INVALID_ID, DMHID dst = DMSG_INVALID_ID);
	static int SimpleCall(const char* szMsgName, void* pParam1 = NULL, void* pParam2 = NULL, void* pParam3 = NULL, 
								 void* pParam4 = NULL, void* pParam5 = NULL, void* pRetParam = NULL, 
								 DMHID src = DMSG_INVALID_ID, DMHID dst = DMSG_INVALID_ID);

	Json::Value m_jsonInValue;							// ͨ�ò������壨json��
	Json::Value m_jsonOutValue;							// ͨ�ò������壨json��

	void*		m_pParam1;								// ͨ�ò������壨�û��Զ���ʹ�ã�
	void*		m_pParam2;								// ͨ�ò������壨�û��Զ���ʹ�ã�
	void* 		m_pParam3;								// ͨ�ò������壨�û��Զ���ʹ�ã�
	void* 		m_pParam4;								// ͨ�ò������壨�û��Զ���ʹ�ã�
	void* 		m_pParam5;								// ͨ�ò������壨�û��Զ���ʹ�ã�
	void*		m_pRetParam;							// ͨ�ò������壨�û�����ֵ��
	/**********************************************************************************************************/ 

	void		SetFlag(int nFlag = DMSG_FLAG_MSG_NORMAL){ m_nFlag = nFlag; };
	DMSG_ACTION GetAction() { return m_actType; }
	uint32_t	MsgSeq(){ return m_nMsgSeq; }			// ��ϢΨһ����
	const char* Trace();								// �������
	void		SetResult(DMSG_RESULT nResult){ m_nResult = nResult; }
	DMSG_RESULT GetResult(){ return m_nResult; }		// ���ش�����
	void		SetStartupTime(uint32_t nStartupTim = 0); // ������Ϣ��ʼʱ�䣬���� ��ʱ����
	void		SetTimeout(uint32_t nTimeout){ m_nTimeout = nTimeout; };
	int64_t		GetTimeout(){ return m_nStartupTime + m_nTimeout; }
	uint32_t	CheckTimeout();							// ���ڳ�ʱ���

	DECLARE_RTTI_CLASS(DMsg);							// ��̬���ͼ��

protected:
	int			PushMsg(bool bSync);
	virtual const char* CustomizedTrace() { return ""; }
	std::string m_sCustomized;
	std::string m_sTrace;

private:
	DMSG_ACTION		m_actType;
	uint32_t		m_nStartupTime;
	uint32_t		m_nTimeout;							// The minimum unit for 5 seconds
	uint32_t		m_nMsgSeq;							// ��Ϣȫ��Ψһ���к�
	int				m_nFlag;							// ����ָ����Ϣ���ȶ�

	static DAtomic	g_nMsgSeq;
	DMSG_RESULT		m_nResult;							// 0 indicate: Unknown
	static DMsgBus* g_pBindMsgBus;						// ���ڰ��ص� �ں� ���á�

	std::string		m_sMsgName;							// ���������ж���Ĭ��Ϊ className
	/**********************************************************************************************************/


	/******************************************������1.0�汾�Ĺ���*********************************************/
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
	int   m_type;  // ��Ϣ���� 0 ~ 999 ��ϵͳ����ֵ
	DMHID m_src;
	DMHID m_dst;
	/**********************************************************************************************************/
};



/**************************************���º�����MSG�����ݴ�ӡ�͸���***************************************/
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


// ���� �̳д�ӡ����
#define BEGIN_MSG_CUSTOMIZED_TRACE_INHERITED_BASECLASS(baseClass)	\
	virtual const char* CustomizedTrace(){ m_sCustomized = baseClass::CustomizedTrace(); \
	m_sCustomized += "  ";	char sBuf[CUSTOMIZED_TRACE_BUF + 1];
/**********************************************************************************************************/



// �˻ظ��û�����Ϣ�������ŵ���ԭʼ��Ϣ
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