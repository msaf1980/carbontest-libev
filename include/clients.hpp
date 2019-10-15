#ifndef _CLIENTS_HPP_
#define _CLIENTS_HPP_

#include <cstdlib>

#include <ev.h>

#include <config.hpp>
#include <netstat.hpp>

#define MAX_MESSAGE_LEN 1024

struct client {
	int sock;
	int socktype;
	char buf[MAX_MESSAGE_LEN];
	ev_io watcher;
};

extern struct client *clients;

void clientThread(const int &thread_id, const int &thread_count);

#endif /* _CLIENTS_HPP_ */
