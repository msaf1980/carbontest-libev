#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <vector>

#include <plog/Log.h>

#include <ev.h>

#include <c_procs/fileutils.h>

#include <clients.hpp>

using std::vector;

struct client *clients;


static void
connect_handler(struct ev_loop *loop, struct ev_io *watcher, int events) {
    //ev_io_stop(loop, &domain->io);
    //ev_timer_stop(loop, &domain->tw);
    int error;
    socklen_t len = sizeof (error);
    if (getsockopt(domain->io.fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error) {
        free_domain(domain);
        errno = error;

    } else {
        //send_request(loop, domain);
		close(wa
    }

}

int newTCPSession(struct ev_loop *loop, int worker, int thread_id) {

	clients[worker].sock = socket(AF_INET, SOCK_STREAM, 0);
	if (clients[worker].sock == -1) {
		LOG_WARNING << "Failed to create TCP socket in " << worker
		            << " in client thread " << thread_id << ":"
		            << strerror(errno);
		return -1;
	}
	set_nonblock(clients[worker].sock);
	LOG_VERBOSE << "Starting TCP client " << worker << " in client thread "
	            << thread_id;

	ev_io_init(clients[worker].watcher,  TCPConnectCb, clients[worker].sock,
	           EV_WRITE);
	ev_io_start(loop, &clients[worker].watcher);

	if (connect(clients[worker].sock,
	            (struct sockaddr *) &clients[worker].servaddr,
	            sizeof(clients[worker].servaddr)) < 0) {
		if (errno != EINPROGRESS)
			return -1;
	}
	return 0;
}

void clientThread(const int &thread_id, const int &thread_count) {
	LOG_VERBOSE << "Starting client thread " << thread_id;
	struct ev_loop *loop = ev_loop_new(0);

	for (int i = thread_id; i < config.Workers; i += thread_count) {
		newTCPSession(loop, i, thread_id);
	}
	for (int i = thread_id; i < config.UWorkers; i += thread_count) {
	}

	LOG_VERBOSE << "Shutdown client thread " << thread_id;

	ev_run(loop, 0);
}
