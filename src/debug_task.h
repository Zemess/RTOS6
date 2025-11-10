#pragma once
#include <zephyr/kernel.h>

// Käynnistetään debug säie.
void debug_task_start(void);

void debug_logf(const char *fmt, ...);

extern volatile bool g_debug_enabled;