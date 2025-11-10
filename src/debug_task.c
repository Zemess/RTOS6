#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "debug_task.h"

// Enabloidaan tai disabloidaan debug logit
volatile bool g_debug_enabled = false;

/* FIFO Rakenne */
struct debug_msg {
    void *fifo_reserved; // Zephyrin sisäinen FIFO-kenttä
    char buf[160];
};

K_FIFO_DEFINE(debug_fifo);

void debug_logf(const char *fmt, ...)
{
    if (!g_debug_enabled) return;

    struct debug_msg *m = k_malloc(sizeof(struct debug_msg));
    if (!m) return;

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(m->buf, sizeof(m->buf), fmt, ap);
    va_end(ap);

    k_fifo_put(&debug_fifo, m);
}

/* Tulostaja */

static void debug_task(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    for (;;) {
        struct debug_msg *m = k_fifo_get(&debug_fifo, K_FOREVER);
        if (!m) continue;
        printk("%s", m->buf);
        k_free(m);
    }
}

/* Määritellään säie. pienempi prioriteetti kuin muilla täskeillä */

#define DEBUG_STACK 1024
#define DEBUG_PRIO  7

K_THREAD_DEFINE(debug_thread, DEBUG_STACK, debug_task, NULL, NULL, NULL,
                DEBUG_PRIO, 0, 0);

void debug_task_start(void)
{
    // Säie käynnistyy automaattisesti K_THREAD_DEFINE makrolla
}