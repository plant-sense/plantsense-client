#pragma once
#include <syslog.h>

#include <csignal>

#ifdef MONIT_DEBUG
#warning "debug ON"
#define debug(X) X
#else
#define debug(X)
#endif

