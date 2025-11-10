#include <zephyr/kernel.h>       // sisältää K_FOREVER, K_FIFO_DEFINE, k_sem jne.
#include <zephyr/sys/printk.h>
#include <string.h>
#include "led_example.h"
#include "dispatcher.h"
#include "debug.h"
#include "timer_action.h"

// Viittaukset LED-taskien semafooreihin (määritelty led_example.c:ssä)
extern struct k_sem red_sem;
extern struct k_sem green_sem;
extern struct k_sem yellow_sem;
extern struct k_sem release_sem;

// Lasketaan perusekvenssien summa

extern volatile uint64_t red_last_us, green_last_us, yellow_last_us;
static int button_press_count = 0;
static char seq_state = 0; // Seuraava väri sekvenssissä: 0=red,1=green,2=yellow

void on_task_done(char which) {
    switch (seq_state) {
    case 0:
        if (which == 'r') {
            seq_state = 1; // seuraavaksi green
        } else {
            seq_state = 0; // pysytään redissä
        }  
        break;

    case 1:
        if (which == 'y') {
            seq_state = 2; // seuraavaksi yellow
        } else {
            seq_state = 0; // palataan rediin
        }  
        break;

    case 2:
        if (which == 'g') {
            uint64_t total_us = red_last_us + green_last_us + yellow_last_us;
            DBG("sequence R->Y->G done, total time: %llu us\n",
                (unsigned long long)total_us);
        }
        seq_state = 0; // Aloitetaan alusta 
        break;

    default:
        seq_state = 0;
        break;

    }
    DBG("Task '%c' done, next in sequence: %d\n", which, seq_state);
}

// FIFO määrittely dispatcherille
K_FIFO_DEFINE(dispatcher_fifo);

// Dispatcherille tapahtuman lähetys FIFOon
void dispatcher_post(dispatcher_event_t *evt) {
    dispatcher_event_t *copy = k_malloc(sizeof(dispatcher_event_t));
    if (copy) {
        memcpy(copy, evt, sizeof(dispatcher_event_t));
        if (evt->type == EVENT_LED_CMD) {
            DBG("Dispatcher: received LED event '%s'\n", evt->msg);
        } else if (evt->type == EVENT_TIMER_START) {
            DBG("Dispatcher: received TIMER event (%d s)\n", evt->seconds);
        }
        k_fifo_put(&dispatcher_fifo, copy);
    } else {
        DBG("Dispatcher: memory alloc failed!\n");
    }
}

// Dispatcher-task, joka lukee FIFOa ja ohjaa valotaskien semafooreja
static void dispatcher_task(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    DBG("Dispatcher task started\n");

    while (1) {
        dispatcher_event_t *evt = k_fifo_get(&dispatcher_fifo, K_FOREVER);
        if (!evt) { continue; }

        // 3a) TIMER-eventit
        if (evt->type == EVENT_TIMER_START) {
            if (evt->seconds > 0) {
                DBG("Dispatcher: starting timer for %d s\n", evt->seconds);
                timer_action_start_seconds(evt->seconds);
            } else {
                DBG("Dispatcher: ignored timer start (seconds=%d)\n", evt->seconds);
            }
            k_free(evt);
            continue;
        }
        // 3b) LED-komennot (EVENT_LED_CMD)
        DBG("Dispatcher: got event -> '%s'\n", evt->msg);
        if (evt->msg[0] == 'r' || evt->msg[0] == 'y' || evt->msg[0] == 'g') {
            button_press_count++;
            DBG("Button press count: %d\n", button_press_count);
        }
        switch (evt->msg[0]) {
            case 'r':
                DBG("Dispatcher: activating RED\n");
                k_sem_take(&release_sem, K_FOREVER);
                k_sem_give(&red_sem);
                break;

            case 'y':
                DBG("Dispatcher: activating YELLOW\n");
                k_sem_take(&release_sem, K_FOREVER);
                k_sem_give(&yellow_sem);
                break;

            case 'g':
                DBG("Dispatcher: activating GREEN\n");
                k_sem_take(&release_sem, K_FOREVER);
                k_sem_give(&green_sem);
                break;

            case 'D':
            case 'd':
                // Varmista että saat aina kuittauksen:
                if (!g_debug_enabled) {
                    g_debug_enabled = true;
                    DBG("Dispatcher: Debug logs ENABLED\n");
                } else {
                    DBG("Dispatcher: Debug logs DISABLED\n");
                    g_debug_enabled = false;
                }
                break;

            default:
                DBG("Dispatcher: unknown symbol '%c'\n", evt->msg[0]);
                break;
        }

         k_free(evt);
    }

}

// Dispatcher-säikeen määrittely
K_THREAD_DEFINE(dispatch_thread, 1024, dispatcher_task, NULL, NULL, NULL, 5, 0, 0);

// Dispatcher-moduulin alustus
void dispatcher_init(void) {
    DBG("Dispatcher initialized\n");
}
