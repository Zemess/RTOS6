// Debug macroja

#pragma once
#include "debug_task.h"

#ifndef DEBUG_PRINTS
#define DEBUG_PRINTS 1
#endif

#if DEBUG_PRINTS
    #define DBG(fmt, ...) debug_logf("DEBUG: " fmt, ##__VA_ARGS__)
#else
    #define DBG(fmt, ...)
#endif