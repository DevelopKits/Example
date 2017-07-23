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

//accept回掉函数
void do_accept_cb(evutil_socket_t listener, short evnet, void* arg);
//read 会掉函数
void read_cd(struct bufferevent*bev, void* arg);
//err回掉函数
void error_cb(struct bufferevent*bev, short event, void* arg);
//write回掉函数
void write_cd(struct bufferevent*bev, void* arg);


void do_accept_cb(evutil_socket_t listener, short evnet, void* arg)
{
	//传入的event_base指针
	struct event_base* base = (struct event_base*)arg;
	//socket描述符
	evutil_socket_t fd;
	//声明地址
	struct sockaddr_in sin;
	//地址长度声明
	socklen_t slen = sizeof(sin);
	fd = accept(listener, (struct sockaddr*)&sin, &slen);
	if (fd < 0)
	{
		perror("error accept");
		return;
	}
	//注册一个bufferevent_socket_new事件
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	//设置回掉函数
	bufferevent_setcb(bev, read_cd, NULL, error_cb, arg);
	//设置该事件的属性
	bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
}

void read_cd(struct bufferevent*bev, void* arg)
{
#define  MAX_LINE 256
	char line[MAX_LINE + 1];
	int n;
	//通过传入参数bev找到socket fd
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
	//通过传入参数bev找到socket fd
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
	//通过传入参数bev找到socket fd
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


