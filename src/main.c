#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/timing/timing.h>
#include "led_example.h"
#include "button_example.h"
#include "dispatcher.h"
#include "uartDef.h"
#include "debug_task.h"
#include "debug.h"

#define	STACKSIZE	1024
#define	PRIORITY	5

/* 2 / 4 pistettä tähtäsin tällä kun Testit ja parseri integroituna sulautettuun softaan. */

// Main program
int main(void) {
    DBG("Koitetaan rullaa softaa.\n");
    // Timerit
    timing_init();
    timing_start();
    // Debug task
    debug_task_start();
    // Uart ja Dispatcher
    uartDef_init();
    dispatcher_init();
    // Muut
    init_button();
    init_led();

    return 0;
}
