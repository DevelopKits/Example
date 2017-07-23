#include "stdafx.h"


#include <WinSock2.h>
#include <iostream>
#include <string.h>
#include <errno.h>

#include "event2/event.h"
#include "event2/bufferevent.h"
#include <WS2tcpip.h>

using namespace std;
#define LISTEN_PORT 9999

//accept�ص�����
void do_accept_cb(evutil_socket_t listener, short evnet, void* arg);
//read �������
void read_cd(struct bufferevent*bev, void* arg);
//err�ص�����
void error_cb(struct bufferevent*bev, short event, void* arg);
//write�ص�����
void write_cd(struct bufferevent*bev, void* arg);


void do_accept_cb(evutil_socket_t listener, short evnet, void* arg)
{
	//�����event_baseָ��
	struct event_base* base = (struct event_base*)arg;
	//socket������
	evutil_socket_t fd;
	//������ַ
	struct sockaddr_in sin;
	//��ַ��������
	socklen_t slen = sizeof(sin);
	fd = accept(listener, (struct sockaddr*)&sin, &slen);
	if (fd < 0)
	{
		perror("error accept");
		return;
	}
	//ע��һ��bufferevent_socket_new�¼�
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	//���ûص�����
	bufferevent_setcb(bev, read_cd, NULL, error_cb, arg);
	//���ø��¼�������
	bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
}

void read_cd(struct bufferevent*bev, void* arg)
{
#define  MAX_LINE 256
	char line[MAX_LINE + 1];
	int n;
	//ͨ���������bev�ҵ�socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	while (n = bufferevent_read(bev, line, MAX_LINE))
	{
		line[n] = '\0';
		cout << "fd = " << fd << " readline = " << line << endl;
		bufferevent_write(bev, line, n);
	}
}

void error_cb(struct bufferevent*bev, short event, void* arg)
{
	//ͨ���������bev�ҵ�socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	if (event&BEV_EVENT_TIMEOUT)
	{
		cout << "Timer out" << endl;
	}
	else if (event&BEV_EVENT_EOF)
	{
		cout << "connection closed" << endl;
	}
	else if (event&&BEV_EVENT_ERROR)
	{
		cout << "some other error" << endl;
	}
	bufferevent_free(bev);
}

void write_cd(struct bufferevent*bev, void* arg)
{
	char str[50] = { 0 };
	//ͨ���������bev�ҵ�socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	cout << "please input something" << endl;
	scanf_s("%s", &str);
	bufferevent_write(bev, &str, sizeof(str));
}

int main()
{
	int iret;
	evutil_socket_t listener;
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
	{
		return -1;
	}
	listener = socket(AF_INET, SOCK_STREAM, 0);
	evutil_make_listen_socket_reuseable(listener);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(LISTEN_PORT);
	if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		perror("bind");
		return -1;
	}
	listen(listener, 10000);
	evutil_make_socket_nonblocking(listener);
	struct event_base* base = event_base_new();
	struct event*listen_event;
	listen_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept_cb, (void*)base);
	event_add(listen_event, NULL);
	event_base_dispatch(base);
	return 0;
}


