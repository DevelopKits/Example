#include "WorkThread.h"


WorkThread::WorkThread()
{
	Start();
	AddMsgFunc(MsgWorK, &WorkThread::OnMsgNotify);
}


WorkThread::~WorkThread()
{
	if (IsRunning())
	{
		Stop();
	}
}

void WorkThread::start(void)
{
	CMediaMessage* pMsg = new CMediaMessage(MsgWorK);
	pMsg->innerMsgType = EM_MdMsg_START;
	pMsg->pData = "start";
	dsl::DRef<dsl::DMessage> msg = pMsg;
	PushMsg(msg.GetPointer());
}

void WorkThread::stop(void)
{
	CMediaMessage* pMsg = new CMediaMessage(MsgWorK);
	pMsg->innerMsgType = EM_MdMsg_STOP;
	pMsg->pData = "stop";
	dsl::DRef<dsl::DMessage> msg = pMsg;
	PushMsg(msg.GetPointer());
}

 void WorkThread::OnMsgNotify(dsl::DMessage* msg)
{
	CMediaMessage* pMediaMsg = static_cast<CMediaMessage*>(msg);
	if (pMediaMsg == NULL)
	{
		return;
	}
	int innerMsgType = pMediaMsg->innerMsgType;
	switch (innerMsgType)
	{
		case EM_MdMsg_START:
		{
			DLOG_INFO("EM_MdMsg_START %s", pMediaMsg->pData);
			break;
		}
		case EM_MdMsg_STOP:
		{
			DLOG_INFO("EM_MdMsg_STOP %s", pMediaMsg->pData);
			break;
		}
	}
}

CMediaMessage::CMediaMessage(int type /*= 0*/)
{
	innerMsgType = 0;
	pData = 0;
}

CMediaMessage::~CMediaMessage()
{
	if (pData)
	{
		delete(pData);
		pData = NULL;
	}
}
