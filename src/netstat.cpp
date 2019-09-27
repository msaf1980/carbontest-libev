#include <netstat.hpp>

const char *NetOperStr[] = {"CONNECT", "SEND", "RECV", NULL};

const char *NetErrStr[] = {"OK",    "ERROR",        "LOOKUP",
                           "PIPE",  "TIMEOUT",      "REFUSED",
                           "RESET", "UNREACHEABLE", NULL};

const char *NetProtoStr[] = {"TCP", "UDP", NULL};
