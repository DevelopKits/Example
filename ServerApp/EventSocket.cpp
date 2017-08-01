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

bool EventSocket::StartServer(int port, unsigned int connnum, int read_timeout, int write_timeout)
{
	m_Server.bStart = false;
	m_Server.nPort = port;
	m_Server.connnum = connnum;
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
	m_Server.pWorker = new Worker;
	struct event_config *pevConfig = event_config_new();
	event_config_set_flag(pevConfig, EVENT_BASE_FLAG_STARTUP_IOCP);
	//根据CPU实际数量配置libEvent的CPU数
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	event_config_set_num_cpus_hint(pevConfig, si.dwNumberOfProcessors);
	m_Server.pWorker->pWokerbase =event_base_new() /*event_base_new_with_config(pevConfig)*/;
	event_config_free(pevConfig);
	if (m_Server.pWorker->pWokerbase == NULL)
	{
		delete m_Server.pWorker;
		return false;
	}
	//初始化连接对象
	m_Server.pWorker->pListConn = new ConnList();
	if (m_Server.pWorker->pListConn == NULL)
	{
		return false;
	}
	m_Server.pWorker->pListConn->plistConn = new Conn[m_Server.connnum + 1];
	m_Server.pWorker->pListConn->head = &m_Server.pWorker->pListConn->plistConn[0];
	m_Server.pWorker->pListConn->tail = &m_Server.pWorker->pListConn->plistConn[m_Server.connnum];
	for (unsigned int j = 0; j < m_Server.connnum; j++)
	{
		m_Server.pWorker->pListConn->plistConn[j].index = j;
		m_Server.pWorker->pListConn->plistConn[j].next = &m_Server.pWorker->pListConn->plistConn[j + 1];
	}
	m_Server.pWorker->pListConn->plistConn[m_Server.connnum].index = m_Server.connnum;
	m_Server.pWorker->pListConn->plistConn[m_Server.connnum].next = NULL;
	//设置当前事件
	Conn *p = m_Server.pWorker->pListConn->head;
	while (p != NULL)
	{
		p->bufev = bufferevent_socket_new(m_Server.pWorker->pWokerbase, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
		bufferevent_setcb(p->bufev, DoRead, NULL, DoError, p);
		bufferevent_setwatermark(p->bufev, EV_READ, 0, MaxBuffLen);
		bufferevent_enable(p->bufev, EV_READ | EV_WRITE);
		
		p->owner = m_Server.pWorker;
		p = p->next;
	}
	m_Server.pWorker->hThread = CreateThread(NULL, 0, ThreadWorkers, m_Server.pWorker, 0, NULL);
	m_Server.hThread = CreateThread(NULL, 0, ThreadServer, &m_Server, 0, NULL);
	if (m_Server.hThread == NULL || m_Server.pWorker->hThread==NULL)
	{
		return false;
	}
	m_Server.bStart = true;
	return true;
}

void EventSocket::StopServer()
{
	
}
void EventSocket::DoAccept(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	char Buffer[256];
	sockaddr_in* addr = (sockaddr_in*)sa;
	evutil_inet_ntop(addr->sin_family, &addr->sin_addr, Buffer, sizeof(Buffer));
	cout << "accept a client,IP: " << Buffer << ",port:" << addr->sin_port << endl;

	//此处为监听线程的event.不做处理.
	Server *pServer = (Server *)user_data;
	Worker *pWorker = pServer->pWorker;
	//通知线程开始读取数据,用于分配哪一个线程来处理此处的event事件
	EnterCriticalSection(&pWorker->cs);
	Conn *pConn = pWorker->GetFreeConn();
	LeaveCriticalSection(&pWorker->cs);
	if (pConn == NULL)
	{
		cout << "more than " << pServer->connnum <<" connects" <<endl;
		return;
	}
	
	pConn->fd = fd;
	evutil_make_socket_nonblocking(pConn->fd);
	bufferevent_setfd(pConn->bufev, pConn->fd);
	bufferevent_enable(pConn->bufev, EV_READ | EV_WRITE);

	struct timeval delayWriteTimeout;
	delayWriteTimeout.tv_sec = pServer->write_timeout;
	delayWriteTimeout.tv_usec = 0;
	struct timeval delayReadTimeout;
	delayReadTimeout.tv_sec = pServer->read_timeout;
	delayReadTimeout.tv_usec = 0;
	bufferevent_set_timeouts(pConn->bufev, &delayReadTimeout, &delayWriteTimeout);
}

void EventSocket::DoRead(struct bufferevent *bev, void *ctx)
{
	Conn *c = (Conn*)ctx;
	struct evbuffer * input = bufferevent_get_input(bev);
	if (evbuffer_get_length(input))
	{
		
		while (evbuffer_get_length(input))
		{
			c->in_buf_len += evbuffer_remove(input, c->in_buf, MaxBuffLen - c->in_buf_len);
		}
		//cout << "Receive data = " << c->in_buf << ",size = " << c->in_buf_len << endl;
	}
	c->in_buf_len = 0;
}

void EventSocket::DoError(struct bufferevent *bev, short error, void *ctx)
{
	Conn *pConn = (Conn*)ctx;
	if (error & BEV_EVENT_EOF)
	{
		cout << pConn->fd << "connection closed" << endl;
	}
	else if (error & BEV_EVENT_ERROR)
	{
		cout << pConn->fd << "some other error" << endl;
	}
	else if (error & BEV_EVENT_TIMEOUT)
	{
		cout << pConn->fd<< "Timed out" << endl;
	}
	pConn->in_buf_len = 0;
	bufferevent_disable(pConn->bufev, EV_READ | EV_WRITE);
	evutil_closesocket(pConn->fd);
	EnterCriticalSection(&pConn->owner->cs);
	pConn->owner->PutFreeConn(pConn);
	LeaveCriticalSection(&pConn->owner->cs);
	
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