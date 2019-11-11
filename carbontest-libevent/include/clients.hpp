#ifndef _CLIENTS_HPP_
#define _CLIENTS_HPP_

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdlib>

#include <event2/event.h>

#include <config.hpp>
#include <netstat.hpp>

#define MAX_MESSAGE_LEN 1024

struct stream_t {
	evutil_socket_t fd;
	struct event_base *ev_base;
	struct event *revent;

	char obuf[MAX_MESSAGE_LEN];
	struct sockaddr_in servaddr;
	int worker;
};

extern struct stream_t *clients;

void clientThread(const int &thread_id, const int &thread_count);

#endif /* _CLIENTS_HPP_ */
