#include <signal.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <vector>

#include <plog/Log.h>

#include <fmt/format.h>

#include <c_procs/daemonutils.h>
#include <c_procs/errors.h>
#include <threads/spinning_barrier.hpp>

#include <clients.hpp>
#include <netstat.hpp>
#include <runner.hpp>

using std::map;
using std::string;
using std::thread;
using std::vector;

const char *NetOperStr[] = {"CONNECT", "SEND", "RECV", NULL};

const char *NetErrStr[] = {"OK",    "ERROR",        "LOOKUP",
                           "PIPE",  "TIMEOUT",      "REFUSED",
                           "RESET", "UNREACHEABLE", NULL};

const char *NetProtoStr[] = {"TCP", "UDP", NULL};

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
