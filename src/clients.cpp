#include <stdlib.h>

#include <vector>

#include <plog/Log.h>

#include <ev.h>

#include <clients.hpp>

using std::vector;

void clientThread(const int &thread_id, const int &thread_count, const Config &config, NetStatQueue &queue) {
	LOG_VERBOSE << "Starting client thread " << thread_id;
	vector<ev_async *> watchers;
	struct ev_loop *loop = ev_loop_new(0);

	for (int i = thread_id; i < config.Workers; i += thread_count) {
		ev_async *w = new ev_async;	
		LOG_VERBOSE << "Starting TCP client " << i << " in client thread " << thread_id;
	}
	for (int i = thread_id; i < config.UWorkers; i += thread_count) {
		LOG_VERBOSE << "Starting UDP client " << i << " in client thread " << thread_id;
	}

	LOG_VERBOSE << "Shutdown client thread " << thread_id;

	ev_run(loop, 0);
}
