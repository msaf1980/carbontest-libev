#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <vector>

#include <plog/Log.h>

#include <event2/event.h>

#include <c_procs/fileutils.h>

#include <clients.hpp>

using std::vector;

struct stream_t *clients;

// void tcp_session(struct ev_loop *loop, client_t *client);
// void tcp_session_reconnect(struct ev_loop *loop, client_t *client);

/*
static void tcp_connect_cb(struct ev_loop *loop, struct ev_io *watcher,
                           int events) {
    int       err;
    client_t *client =
        (client_t *) (((char *) watcher) - offsetof(client_t, watcher));
    ev_io_stop(loop, watcher);

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
*/

/*
static void tcp_reconnect_cb(struct ev_loop *loop, struct ev_timer *watcher,
                             int events) {
    client_t *client =
        (client_t *) (((char *) watcher) - offsetof(client_t, timer));
    tcp_session(loop, client);
}
*/

/*
static void tcp_timeout_cb(struct ev_loop *loop, struct ev_timer *watcher,
                           int events) {
                        */
/* connection timeout occurred */
/*
    client_t *client =
        (client_t *) (((char *) watcher) - offsetof(client_t, timer));

    ev_io_stop(loop, &client->watcher);

    if (client->watcher.fd > 0) {
        close(client->watcher.fd);
        LOG_WARNING << "Failed to connect TCP socket in worker "
                    << client->worker << ": timeout";
    }
*/
/*
if (config.Delay > 0) {
    ev_timer_init(&client->timer, tcp_reconnect_cb, config.Delay, 0);
    ev_timer_start(loop, &client->timer);
} else {
    tcp_session(loop, client);
}
*/
//}


int tcp_connect_tout(struct event_base * ev_base, stream_t *client,
                     struct sockaddr *addr, socklen_t addrlen, double timeout) {
	struct timeval connect_tout;
    //client->loop = loop;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return -1;
    }
    set_nonblock(fd);

    if (connect(fd, addr, addrlen) < 0) {
        if (errno != EINPROGRESS && errno != EWOULDBLOCK) {
            return -1;
        }
    }

    return 0;
}


void tcp_session(struct event_base * ev_base, stream_t * client) {
	struct event *ev;

    if (!running.load())
        return;

/*
if (tcp_connect_tout(loop, client, (sockaddr *) config.address->ai_addr,
                     config.address->ai_addrlen, config.ConTimeout) < 0) {
    if (errno != EINPROGRESS) {
        LOG_WARNING << "Failed to connect TCP socket in worker "
                    << client->worker << ": " << strerror(errno);
        // tcp_session_reconnect(loop, client);
    }
}
*/

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
	struct event_base *ev_base;
	struct event_config *ev_config = event_config_new();
	if (!ev_config) {
		LOG_ERROR << "Alloc eventloop in thread " << thread_id;
		return;
	}
	ev_base = event_base_new_with_config(ev_config);
	if (!ev_base) {
		event_config_free(ev_config);
		LOG_ERROR << "Alloc eventloop in thread " << thread_id;
		return;
	}
	LOG_VERBOSE << "Starting client thread " << thread_id;

	event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);

	int worker = 0;
	for (int i = thread_id; i < config.Workers; i += thread_count) {
		clients[worker].worker = worker;
		LOG_VERBOSE << "Starting TCP worker " << worker;
		tcp_session(ev_base, &clients[worker]);
		worker++;
	}
	for (int i = thread_id; i < config.UWorkers; i += thread_count) {
		worker++;
	}

	LOG_VERBOSE << "Shutdown client thread " << thread_id;

	event_base_dispatch(ev_base);

	event_base_free(ev_base);
	event_config_free(ev_config);
}
