/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* ESBProtocal.h : Э�鶨��
* ����    ��������
* ������ڣ�2014��9��28��
*
* ��ǰ�汾��1.0
*/

#pragma once

#include <libdsl/DHttp.h>

#define LOG_PROTO(traderId, pHttp, LOG_LEVEL)  ( dsl::DPrintLog::instance()->Log( __FILE__, __LINE__, DLOG_MODULE, LOG_LEVEL, \
												"[Client] TraderId[%s] %s[%s] %s[%s] %s[%s] %s[%s] %s[%s] %s[%s]", traderId, \
												ESB_PROTO_FROM, pHttp->GetHeader(ESB_PROTO_FROM), \
												ESB_PROTO_TO, pHttp->GetHeader(ESB_PROTO_TO), \
												ESB_PROTO_CSEQ, pHttp->GetHeader(ESB_PROTO_CSEQ), \
												ESB_PROTO_CMD, pHttp->GetHeader(ESB_PROTO_CMD), \
												ESB_PROTO_ACTION, pHttp->GetHeader(ESB_PROTO_ACTION), \
												ESB_PROTO_DISPATCH, pHttp->GetHeader(ESB_PROTO_DISPATCH)) )

#define DEBUG_LOG_PROTO(traderId, pHttp)		LOG_PROTO(traderId, pHttp, DLOG_LEVEL_DEBUG)
#define INFO_LOG_PROTO(traderId, pHttp)			LOG_PROTO(traderId, pHttp, DLOG_LEVEL_INFO)
#define ERR_LOG_PROTO(traderId, pHttp)			LOG_PROTO(traderId, pHttp, DLOG_LEVEL_ERR)

#define ESB_NET_THREAD_NUM			16				// ESB�����߳���
#define ESB_SERVICE					"ESBService"	// ESB������


#define ESB_PROTO_PARSER			"EsbParser"		// ESB�Զ��������
#define ESB_PROTO_DISTRIBUTE		"EsbDistribute"	// Э��ַ�����

/***********************************HTTP Э��ͷ*****************************************/
#define ESB_PROTO_FROM				"From"
#define ESB_PROTO_TO				"To"
#define ESB_PROTO_VIA				"Via"
#define ESB_PROTO_CMD_TIMEOUT		"CmdTimeout"
#define ESB_PROTO_CSEQ				"CSeq"
#define ESB_PROTO_CMD				"Cmd"

#define ESB_PROTO_ACTION			"Action"
#define ESB_PROTO_ACTION_REQ		"Request"
#define ESB_PROTO_ACTION_RSP		"Response"
#define ESB_PROTO_ACTION_ACK		"Ack"

#define ESB_PROTO_DISPATCH			"Dispatch"
#define ESB_PROTO_ERRNO				"ErrNo"
#define ESB_PROTO_ERRMSG			"ErrMsg"
/****************************************************************************************/

/***********************************ESB ϵͳЭ��*****************************************/
#define ESB_PROTO_REGISTER			"ESBRegister"
#define ESB_PROTO_QUIT				"ESBQuit"


/****************************************************************************************/


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// HTTP�� ����
class LIBDSL_API ESBProtoHelper
{
public:
	static int CreateRequestHttp(DRef<DHttp> &pHttp, const char* szCmd, const char* szTo, const char* szDispatch);
	static int CreateResponseHttp(DRef<DHttp> &pRspHttp, DHttp* pReqHttp, int nErrNo, const char* szErrMsg);
	static int CreateAckHttp(DRef<DHttp> &pAckHttp, DHttp* pRspHttp);

private:
	static DAtomic m_atmSeq;
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL