#include <event2/bufferevent.h>
#include <event2/bufferevent_compat.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer_compat.h>
#include <event2/http_struct.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>

#define MaxBuffLen 4096


struct _Server ;
struct _Worker ;
struct _Conn ;
struct _ConnList ;

// ���Ӷ���
struct _Conn
{
	_Conn()
	{
		fd = NULL;
		bufev = NULL;
		index = -1;
		in_buf_len = 0;
		out_buf_len = 0;
		owner = NULL;
		next = NULL;
		in_buf = new char[MaxBuffLen];
		out_buf = new char[MaxBuffLen];
	}
	~_Conn()
	{
		delete[]in_buf;
		delete[]out_buf;
		bufferevent_free(bufev);
	}
	struct bufferevent *bufev;
	evutil_socket_t fd;
	int index;
	char *in_buf;
	short in_buf_len;
	char *out_buf;
	short out_buf_len;
	_Worker *owner;
	_Conn *next;
};

//���Ӷ����б�
struct _ConnList
{
	_Conn *head;
	_Conn *tail;
	_Conn *plistConn;
	_ConnList()
	{
		head = NULL;
		tail = NULL;
		plistConn = NULL;
	}
	
};


//�����̷߳�װ����.
struct _Worker
{
	_Worker()
	{
		pWokerbase = NULL;
		hThread = INVALID_HANDLE_VALUE;
		pListConn = NULL;
	}
	struct event_base *pWokerbase;
	HANDLE hThread;
	_ConnList *pListConn;
	inline _Conn* GetFreeConn()
	{
		_Conn*pItem = NULL;
		if (pListConn->head != pListConn->tail)
		{
			pItem = pListConn->head;
			pListConn->head = pListConn->head->next;
		}
		return pItem;
	}
	inline void PutFreeConn(_Conn *pItem)
	{
		pListConn->tail->next = pItem;
		pListConn->tail = pItem;
	}
};

//���������Է�װ����
struct _Server
{
	bool bStart;
	short nPort;
	short workernum;
	unsigned int connnum;
	volatile int nCurrentWorker;
	int read_timeout;
	int write_timeout;
	struct evconnlistener *pListener;
	struct event_base *pBase;
	HANDLE hThread;
	_Worker *pWorker;
};

typedef struct _Server Server;
typedef struct _Worker Worker;
typedef struct _Conn Conn;
typedef struct _ConnList ConnList;