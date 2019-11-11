#include <signal.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <vector>

#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Log.h>

#include <fmt/format.h>

#include <c_procs/errors.h>
#include <c_procs/daemonutils.h>

#include <clients.hpp>
#include <netstat.hpp>
#include <runner.hpp>

using std::map;
using std::string;
using std::thread;
using std::vector;

chrono_clock start, end;

struct Thread {
	thread *t;
	int id;
};

int runClients() {
	using float_seconds = std::chrono::duration<double>;
	int rc = 0;
	int thread_count = config.Threads - 1;
	double duration;
	thread *thread_q;

	vector<struct Thread> threads; /* client thread */
	threads.resize(thread_count);


	static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(config.LogLevel, &consoleAppender);
	if (ignore_sigpipe() == -1) {
		LOG_FATAL << "Failed to ignore SIGPIPE: "
		          << strerror2(errno, errbuf, ERRBUF_SIZE);
		GO_LABEL(-1, CLEANUP);
	}

	struct addrinfo hint;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	if ((rc = getaddrinfo(config.Host.c_str(), config.Port.c_str(), &hint, &config.address)) != 0) {
		LOG_FATAL << "Failed getaddrinfo: " << gai_strerror(rc);
		GO_LABEL(-1, CLEANUP);
	}

	clients = new struct stream_t[config.Workers + config.UWorkers];
	if (clients == NULL) {
		LOG_FATAL << "Failed to allocate clients memory";
		GO_LABEL(-1, CLEANUP);
	}

	LOG_INFO << "Starting with " << config.Workers << " TCP clients and "
	         << config.UWorkers << " UDP clients";
	LOG_INFO << "Client thread count " << thread_count;

	running_queue.store(true);
	running.store(true);

	thread_q = new thread(dequeueThread);
	queue_wait.wait();
	if (!running.load()) {
		GO_LABEL(-1, CLEANUP);
	}

	for (int i = 0; i < thread_count; i++) {
		threads[i].id = i;
		threads[i].t = new thread(clientThread, std::ref(threads[i].id),
		                          std::ref(thread_count));
	}

	start = TIME_NOW;
	std::this_thread::sleep_for(float_seconds(config.Timeout));


	LOG_INFO << "Shutting down";
	running.store(false);

	for (int i = 0; i < thread_count; i++) {
		threads[i].t->join();
		delete threads[i].t;
	}
	end = TIME_NOW;
	delete []clients;

	running_queue.store(false);
	thread_q->join();
	delete thread_q;

	duration =
	    std::chrono::duration_cast<float_seconds>(end - start).count();
	if (duration > 0) {
		std::cout << std::fixed;
		std::cout << "Test duration " << duration << " s" << std::endl;
		for (auto &it : stat_count) {
			std::cout << it.first << ": " << it.second << " ("
			          << (double) it.second / duration << " op/s)" << std::endl;
		}
	}
CLEANUP:
	freeaddrinfo(config.address);
	return rc;
}
