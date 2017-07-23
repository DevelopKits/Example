/*
 * Copyright (c) 2000-2007 Niels Provos <provos@citi.umich.edu>
 * Copyright (c) 2007-2012 Niels Provos and Nick Mathewson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _EVENT2_EVENT_STRUCT_H_
#define _EVENT2_EVENT_STRUCT_H_

/** @file event2/event_struct.h

  Structures used by event.h.  Using these structures directly WILL harm
  forward compatibility: be careful.

  No field declared in this file should be used directly in user code.  Except
  for historical reasons, these fields would not be exposed at all.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <event2/event-config.h>
#ifdef _EVENT_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef _EVENT_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/* For int types. */
#include <event2/util.h>

/* For evkeyvalq */
#include <event2/keyvalq_struct.h>

#define EVLIST_TIMEOUT	0x01
#define EVLIST_INSERTED	0x02
#define EVLIST_SIGNAL	0x04
#define EVLIST_ACTIVE	0x08
#define EVLIST_INTERNAL	0x10
#define EVLIST_INIT	0x80

/* EVLIST_X_ Private space: 0x1000-0xf000 */
#define EVLIST_ALL	(0xf000 | 0x9f)

/* Fix so that people don't have to run with <sys/queue.h> */
#ifndef TAILQ_ENTRY
#define _EVENT_DEFINED_TQENTRY
#define TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}
#endif /* !TAILQ_ENTRY */

#ifndef TAILQ_HEAD
#define _EVENT_DEFINED_TQHEAD
#define TAILQ_HEAD(name, type)			\
struct name {					\
	struct type *tqh_first;			\
	struct type **tqh_last;			\
}
#endif

struct event_base;
struct event {
	TAILQ_ENTRY(event) ev_active_next;//激活队列
	TAILQ_ENTRY(event) ev_next;//注册事件队列
	/* for managing timeouts */
	union {
		TAILQ_ENTRY(event) ev_next_with_common_timeout;
		int min_heap_idx;//标志该event结构体在堆中的位置
	} ev_timeout_pos;//仅用于定时事件处理器 EV_TIMEOUT
	//对于IO,是文件描述符(句柄)，对于signal 是信号值
	evutil_socket_t ev_fd;

	//所属的event_base
	struct event_base *ev_base;

	//因为信号和IO是不能同时设置的，所以可以使用union节省内存
	union {
	//无论是信号还是IO,都有一个TAILQ_ENTRY的队列，他用于这样场景
	//用户对于同一个fd调用event_new多次，并且使用了不同的回掉函数
	//每次调用event_new都会产生一个event*,这xxx_next成员就是把这些event链接起来的
		/* 用于IO事件 */
		struct {
			TAILQ_ENTRY(event) ev_io_next;
			struct timeval ev_timeout;
		} ev_io;

		/* 用于信号事件 */
		struct {
			TAILQ_ENTRY(event) ev_signal_next;
			short ev_ncalls;//事件就绪执行时，调用ev_callback的次数
			/* Allows deletes in callback */
			short *ev_pncalls;
		} ev_signal;
	} _ev;

	short ev_events;//关注的事件类型，三种类型io事件(EV_WRITE和EV_READ)，信号(EV_SIGNAL)和定时(EV_TIMEOUT) 辅助选项EV_PERSISR 表明是一个永久事件
	short ev_res;  //记录了当前激活事件的类型
	short ev_flags; //表明当前的状态，可能值为EVLIST_XXX
	ev_uint8_t ev_pri;	//优先级，越小越高，通过event_priority_set设置
	ev_uint8_t ev_closure;
	struct timeval ev_timeout;//用于定时器，设置超时时间

	/* allows us to adopt for different types of events */
	void (*ev_callback)(evutil_socket_t, short, void *arg);//回掉函数
	void *ev_arg;//回掉函数的参数
};

TAILQ_HEAD (event_list, event);

#ifdef _EVENT_DEFINED_TQENTRY
#undef TAILQ_ENTRY
#endif

#ifdef _EVENT_DEFINED_TQHEAD
#undef TAILQ_HEAD
#endif

#ifdef __cplusplus
}
#endif

#endif /* _EVENT2_EVENT_STRUCT_H_ */
