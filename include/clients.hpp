#ifndef _CLIENTS_HPP_
#define _CLIENTS_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <cstdlib>

#include <ev.h>

#include <config.hpp>
#include <netstat.hpp>

#define MAX_MESSAGE_LEN 1024

struct client_t {
	//int socktype;
	char buf[MAX_MESSAGE_LEN];
	struct ev_loop *loop;
	ev_io watcher;
	ev_timer timer;
	struct sockaddr_in servaddr;
	int worker;
};

extern struct client_t *clients;

void clientThread(const int &thread_id, const int &thread_count);

#endif /* _CLIENTS_HPP_ */
