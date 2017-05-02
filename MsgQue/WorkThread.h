#pragma once
#include "libdsl/DMessageQueue.h"
#include "libdsl/DPrintLog.h"
#include "libdsl/DRefObj.h"

enum WorkMSG
{
	MsgWorK = 100,
};

typedef enum _EM_MdMsg
{
	EM_MdMsg_START = 200,
	EM_MdMsg_STOP = 201,
}EM_MdMsg;

class CMediaMessage :public dsl::DMessage
{
public:
	CMediaMessage(int type = 0);
	~CMediaMessage();

	int innerMsgType;
	char* pData;
};

class WorkThread:public dsl::DMessageQueueTpl<WorkThread>
{
public:
	WorkThread();
	~WorkThread();
	void start(void);
	void stop(void);
private:
	void OnMsgNotify(dsl::DMessage* msg);
};

