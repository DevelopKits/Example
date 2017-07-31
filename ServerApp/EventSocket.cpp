#include "EventSocket.h"

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib,"wsock32.lib")

EventSocket::EventSocket(void)
{
	WSADATA WSAData;
	WSAStartup(0x0201, &WSAData);
}

EventSocket::~EventSocket(void)
{
	WSACleanup();
}

bool EventSocket::StartServer(int port, short workernum, unsigned int MaxConNum, int read_timeout, int write_timeout)
{
	m_Server.bStart = false;
	m_Server.nCurrentWorker = 0;
	m_Server.nPort = port;
	m_Server.workernum = workernum;
	m_Server.connnum = MaxConNum;
	m_Server.read_timeout = read_timeout;
	m_Server.write_timeout = write_timeout;
	evthread_use_windows_threads();
	m_Server.pBase = event_base_new();

	if (m_Server.pBase == NULL)
	{
		return false;
	}
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(m_Server.nPort);
	m_Server.pListener = evconnlistener_new_bind(m_Server.pBase, DoAccept, (void*)&m_Server, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&sin, sizeof(sin));
	if (m_Server.pListener == NULL)
	{
		return false;
	}
	m_Server.pWorker = new Worker[workernum];

	for (int i = 0; i < workernum; i++)
	{
		struct event_config *pevConfig = event_config_new();
		event_config_set_flag(pevConfig, EVENT_BASE_FLAG_STARTUP_IOCP);
		m_Server.pWorker[i].pWokerbase = event_base_new_with_config(pevConfig);
		event_config_free(pevConfig);
		if (m_Server.pWorker[i].pWokerbase == NULL)
		{
			delete[]m_Server.pWorker;
			return false;
		}
		//初始化连接对象
		{
			m_Server.pWorker[i].pListConn = new ConnList();
			if (m_Server.pWorker[i].pListConn == NULL)
			{
				return false;
			}
			m_Server.pWorker[i].pListConn->plistConn = new Conn[m_Server.connnum + 1];
			m_Server.pWorker[i].pListConn->head = &m_Server.pWorker[i].pListConn->plistConn[0];
			m_Server.pWorker[i].pListConn->tail = &m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum];
			for (unsigned int j = 0; j < m_Server.connnum; j++) 
			{
				m_Server.pWorker[i].pListConn->plistConn[j].index = j;
				m_Server.pWorker[i].pListConn->plistConn[j].next = &m_Server.pWorker[i].pListConn->plistConn[j + 1];
			}
			m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum].index = m_Server.connnum;
			m_Server.pWorker[i].pListConn->plistConn[m_Server.connnum].next = NULL;
			//设置当前事件
			Conn *p = m_Server.pWorker[i].pListConn->head;
			while (p != NULL)
			{
				p->bufev = bufferevent_socket_new(m_Server.pWorker[i].pWokerbase, -1, BEV_OPT_CLOSE_ON_FREE);
				if (p->bufev == NULL)
				{
					return false;
				}
				bufferevent_setcb(p->bufev, DoRead, NULL, DoError, p);
				bufferevent_setwatermark(p->bufev, EV_READ, 0, MaxBuffLen);
				//bufferevent_enable(p->bufev, EV_READ | EV_WRITE);
				
				p->owner = &m_Server.pWorker[i];
				p = p->next;
			}
		}
		m_Server.pWorker[i].hThread = CreateThread(NULL, 0, ThreadWorkers, &m_Server.pWorker[i], 0, NULL);
	}
	m_Server.hThread = CreateThread(NULL, 0, ThreadServer, &m_Server, 0, NULL);
	if (m_Server.hThread == NULL)
	{
		return false;
	}
	m_Server.bStart = true;
	return true;
}

void EventSocket::StopServer()
{
	if (m_Server.bStart)
	{
		struct timeval delay = { 2, 0 };
		event_base_loopexit(m_Server.pBase, &delay);
		WaitForSingleObject(m_Server.hThread, INFINITE);
		if (m_Server.pWorker)
		{
			for (int i = 0; i < m_Server.workernum; i++)
			{
				event_base_loopexit(m_Server.pWorker[i].pWokerbase, &delay);
				WaitForSingleObject(m_Server.pWorker[i].hThread, INFINITE);
			}
			for (int i = 0; i < m_Server.workernum; i++)
			{
				if (m_Server.pWorker[i].pListConn)
				{
					delete[]m_Server.pWorker[i].pListConn->plistConn;
					delete m_Server.pWorker[i].pListConn;
					m_Server.pWorker[i].pListConn = NULL;
				}
				event_base_free(m_Server.pWorker[i].pWokerbase);
			}
			delete[]m_Server.pWorker;
			m_Server.pWorker = NULL;
		}
		evconnlistener_free(m_Server.pListener);
		event_base_free(m_Server.pBase);
	}
	m_Server.bStart = false;
}

void EventSocket::DoAccept(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	// 此处为监听线程的event.不做处理.
	Server *pServer = (Server *)user_data;
	//主线程处做任务分发.
	int nCurrent = pServer->nCurrentWorker++%pServer->workernum;
	//当前线程所在ID号
	Worker &pWorker = pServer->pWorker[nCurrent];
	//通知线程开始读取数据,用于分配哪一个线程来处理此处的event事件
	Conn *pConn = pWorker.GetFreeConn();
	if (pConn == NULL)
	{
		return;
	}
	pConn->fd = fd;
	evutil_make_socket_nonblocking(pConn->fd);
	bufferevent_setfd(pConn->bufev, pConn->fd);

	struct timeval delayWriteTimeout;
	delayWriteTimeout.tv_sec = pServer->write_timeout;
	delayWriteTimeout.tv_usec = 0;
	struct timeval delayReadTimeout;
	delayReadTimeout.tv_sec = pServer->read_timeout;
	delayReadTimeout.tv_usec = 0;
	bufferevent_set_timeouts(pConn->bufev, &delayReadTimeout, &delayWriteTimeout);
	bufferevent_enable(pConn->bufev, EV_READ | EV_WRITE);
}

void EventSocket::DoRead(struct bufferevent *bev, void *ctx)
{
	struct evbuffer * input = bufferevent_get_input(bev);
	if (evbuffer_get_length(input))
	{
	
	}

}

void EventSocket::DoError(struct bufferevent *bev, short error, void *ctx)
{

	Conn *pConn = (Conn*)ctx;
	pConn->in_buf_len = 0;
	bufferevent_disable(pConn->bufev, EV_READ | EV_WRITE);
	evutil_closesocket(pConn->fd);
	pConn->owner->PutFreeConn(pConn);
}

DWORD WINAPI EventSocket::ThreadServer(LPVOID lPVOID)
{
	Server * pServer = reinterpret_cast<Server *>(lPVOID);
	if (pServer == NULL)
	{
		return -1;
	}
	event_base_dispatch(pServer->pBase);
	return GetCurrentThreadId();
}

DWORD WINAPI EventSocket::ThreadWorkers(LPVOID lPVOID)
{
	Worker *pWorker = reinterpret_cast<Worker *>(lPVOID);
	if (pWorker == NULL)
	{
		return -1;
	}
	event_base_dispatch(pWorker->pWokerbase);
	return GetCurrentThreadId();
}