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

#include <c_procs/daemonutils.h>
#include <threads/spinning_barrier.hpp>

#include <clients.hpp>
#include <netstat.hpp>
#include <runner.hpp>

using std::map;
using std::string;
using std::thread;
using std::vector;

chrono_clock start, end;
map<string, uint64_t> stat_count;

std::atomic_bool running_queue; // running dequeue flag
SpinningBarrier queue_wait(2);

struct Thread {
	thread *t;
	int id;
};

NetStatQueue queue;

void dequeueStat(std::fstream &file) {
	NetStat stat;
	while (queue.try_dequeue(stat)) {
		string name = fmt::format("{}.{}.{}", NetProtoStr[stat.Proto],
		                          NetOperStr[stat.Type], NetErrStr[stat.Error]);
		stat_count[name]++;

		file << stat.TimeStamp << "\t" << stat.Id << "\t"
		     << NetProtoStr[stat.Proto] << "\t" << NetOperStr[stat.Type] << "\t"
		     << NetErrStr[stat.Error] << "\t" << stat.Elapsed << "\t"
		     << stat.Size << "\n";

		if (file.fail()) {
			throw std::runtime_error(config.StatFile + " " + strerror(errno));
		}
	}
}

void dequeueThread() {
	LOG_VERBOSE << "Starting dequeue thread";
	try {
		std::fstream file;
		file.open(config.StatFile, std::ios_base::in);
		if (file.good()) {
			file.close();
			throw std::runtime_error(config.StatFile + " already exist");
		}
		file.open(config.StatFile, std::ios_base::out);
		if (file.fail()) {
			throw std::runtime_error(config.StatFile + " " + strerror(errno));
		}
		file << "Timestamp\tConId\tProto\tType\tStatus\tElapsed(us)\tSize\n";
		if (file.fail()) {
			throw std::runtime_error(config.StatFile + " " + strerror(errno));
		}

		queue_wait.wait();
		while (running_queue.load()) {
			dequeueStat(file);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		file.close();
	} catch (std::exception &e) {
		running.store(false);
		queue_wait.wait();
		// fatal error
		LOG_FATAL << "dequeue thread: " << e.what();
	}
	LOG_VERBOSE << "Shutdown dequeue thread";
}

int runClients() {
	int thread_count = config.Threads - 1;

	vector<struct Thread> threads; /* client thread */
	threads.resize(thread_count);


	static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(config.LogLevel, &consoleAppender);
	if (ignore_sigpipe() == -1) {
		LOG_FATAL << "Failed to ignore SIGPIPE, but failed: "
		          << strerror(errno);
		return -1;
	}
	clients = new struct client[config.Workers + config.UWorkers];
	if (clients == NULL) {
		LOG_FATAL << "Failed to allocate clients memory";
		return -1;
	}

	LOG_INFO << "Starting with " << config.Workers << " TCP clients and "
	         << config.UWorkers << " UDP clients";
	LOG_INFO << "Client thread count " << thread_count;

	running_queue.store(true);
	running.store(true);

	thread *thread_q = new thread(dequeueThread);
	queue_wait.wait();
	if (!running.load())
		return 1;

	for (int i = 0; i < thread_count; i++) {
		threads[i].id = i;
		threads[i].t = new thread(clientThread, std::ref(threads[i].id),
		                          std::ref(thread_count));
	}

	start = TIME_NOW;
	std::this_thread::sleep_for(std::chrono::milliseconds(config.Timeout));

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

	using float_seconds = std::chrono::duration<double>;
	auto duration =
	    std::chrono::duration_cast<float_seconds>(end - start).count();
	if (duration > 0) {
		std::cout << std::fixed;
		std::cout << "Test duration " << duration << " s" << std::endl;
		for (auto &it : stat_count) {
			std::cout << it.first << ": " << it.second << " ("
			          << (double) it.second / duration << " op/s)" << std::endl;
		}
	}

	return 0;
}
