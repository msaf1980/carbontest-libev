#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <vector>

#include <plog/Log.h>

#include <ev.h>

#include <c_procs/fileutils.h>

#include <clients.hpp>

using std::vector;

struct client_t *clients;

void tcp_session(struct ev_loop *loop, client_t *client);
void tcp_session_reconnect(struct ev_loop *loop, client_t *client);

static void tcp_connect_cb(struct ev_loop *loop, struct ev_io *watcher,
                           int events) {
	client_t *client =
	    (client_t *) (((char *) watcher) - offsetof(client_t, watcher));
	//	if (client->watcher.active) {
	ev_io_stop(loop, watcher);
	//ev_timer_stop(loop, &client->timer);
	//	}
	int       err;
	socklen_t len = sizeof(err);
	if (getsockopt(watcher->fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0 ||
	    err) {
		if (err == 0)
			err = errno;
		LOG_WARNING << "Failed to connect TCP socket in worker "
		            << client->worker << ": " << strerror(err);
	} else {
		// send_request(loop, domain);
		LOG_VERBOSE << "Connect TCP socket in worker " << client->worker;
		// ev_io_stop(loop, &client->watcher);
		// ev_timer_stop(loop, &client->timer);
		if (watcher->fd > 0)
			close(watcher->fd);
	}
	// tcp_session_reconnect(loop, client);
}

/*
static void tcp_reconnect_cb(struct ev_loop *loop, struct ev_timer *watcher,
                             int events) {
	client_t *client =
	    (client_t *) (((char *) watcher) - offsetof(client_t, timer));
	tcp_session(loop, client);
}
*/

static void tcp_timeout_cb(struct ev_loop *loop, struct ev_timer *watcher,
                           int events) {
	/* connection timeout occurred */
	client_t *client =
	    (client_t *) (((char *) watcher) - offsetof(client_t, timer));

	ev_io_stop(loop, &client->watcher);

	if (client->watcher.fd > 0) {
		close(client->watcher.fd);
		LOG_WARNING << "Failed to connect TCP socket in worker "
		            << client->worker << ": timeout";
	}

	/*
	if (config.Delay > 0) {
	    ev_timer_init(&client->timer, tcp_reconnect_cb, config.Delay, 0);
	    ev_timer_start(loop, &client->timer);
	} else {
	    tcp_session(loop, client);
	}
	*/
}

int tcp_connect_tout(struct ev_loop *loop, client_t *client,
                     struct sockaddr *addr, socklen_t addrlen, double timeout) {
	client->loop = loop;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		return -1;
	}
	set_nonblock(fd);

	ev_io_init(&client->watcher, tcp_connect_cb, fd, EV_WRITE);
	ev_io_start(loop, &client->watcher);

	//ev_timer_init(&client->timer, tcp_timeout_cb, timeout, 0);
	//ev_timer_start(loop, &client->timer);

	if (connect(fd, addr, addrlen) < 0) {
		if (errno != EINPROGRESS) {
			return -1;
		}
	}

	return 0;
}

void tcp_session(struct ev_loop *loop, client_t *client) {
	if (!running.load())
		return;
	if (tcp_connect_tout(loop, client, (sockaddr *) config.address->ai_addr,
	                     config.address->ai_addrlen, config.ConTimeout) < 0) {
		if (errno != EINPROGRESS) {
			LOG_WARNING << "Failed to connect TCP socket in worker "
			            << client->worker << ": " << strerror(errno);
			// tcp_session_reconnect(loop, client);
		}
	}
}

/*
void tcp_session_reconnect(struct ev_loop *loop, client_t *client) {
	if (config.Delay > 0) {
		ev_timer_init(&client->timer, tcp_reconnect_cb, config.Delay, 0);
		ev_timer_start(loop, &client->timer);
	} else {
		tcp_session(loop, client);
	}
}
*/

void clientThread(const int &thread_id, const int &thread_count) {
	int rc = 0;
	LOG_VERBOSE << "Starting client thread " << thread_id;
	struct ev_loop *loop = ev_loop_new(0);

	int worker = 0;
	for (int i = thread_id; i < config.Workers; i += thread_count) {
		clients[worker].worker = worker;
		LOG_VERBOSE << "Starting TCP worker " << worker;
		tcp_session(loop, &clients[worker]);
		worker++;
	}
	for (int i = thread_id; i < config.UWorkers; i += thread_count) {
		worker++;
	}

	LOG_VERBOSE << "Shutdown client thread " << thread_id;

	ev_run(loop, 0);
}
