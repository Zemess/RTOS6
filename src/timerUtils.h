// Timerin apufunktiot

#pragma once
#include <zephyr/timing/timing.h>

static inline uint64_t nowCyc(void) {
    return timing_counter_get();
}

static inline uint64_t cycToNs(uint64_t cycles) {
    return timing_cycles_to_ns(cycles);
}

static inline uint64_t cycToUs(uint64_t cycles) {
    return timing_cycles_to_ns(cycles) / 1000;
}
