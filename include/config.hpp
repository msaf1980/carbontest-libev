#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include <netdb.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <atomic>
#include <string>

#include <plog/Severity.h>

#include <c_procs/portability.h>
#include <c_procs/errors.h>

struct Config {
	std::string      Host;
	std::string      Port;
	struct addrinfo *address;

	int Duration; // Test duration in seconds

	int Threads; // Threads count

	int Workers;      // TCP Workers
	int MetricPerCon; // Metrics, sended in one connection (TCP)

	int UWorkers; // UDP Workers

	// RateLimit    []int32
	double Delay; // Send/connect delay in seconds

	double ConTimeout; // Connection timeout (seconds)
	double Timeout;    // Send timeout (seconds)

	std::string MetricPrefix; // Prefix for generated metric name

	plog::Severity LogLevel;

	std::string StatFile; // write connections stat to file
};

void parseArgs(Config &config, int argc, char *argv[]);

extern std::atomic_bool running; // running flag
extern Config           config;
#define ERRBUF_SIZE 1024
extern __thread char errbuf[ERRBUF_SIZE];

#endif /* _CONFIG_HPP_ */
