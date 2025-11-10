#include "timer_action.h"
#include <zephyr/kernel.h>
#include "led_example.h"
#include "debug.h"

// Ajastimen ISR-käsittelijä
static void action_timer_handler(struct k_timer *tmr) {
    ARG_UNUSED(tmr);
    // Esimerkkitoiminto: käynnistä punainen LED -taski
    extern struct k_sem red_sem;
    k_sem_give(&red_sem);
    DBG("Timer fired -> RED task triggered\n");
}

// Kertaluonteinen ajastin
K_TIMER_DEFINE(action_timer, action_timer_handler, NULL);

void timer_action_start_seconds(int seconds) {
    if (seconds > 0) {
        k_timer_start(&action_timer, K_SECONDS(seconds), K_NO_WAIT);
        DBG("Timer set for %d s\n", seconds);
    } else {
        DBG("Timer ignored, seconds=%d\n", seconds);
    }
}
