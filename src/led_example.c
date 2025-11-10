#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include "led_example.h"
#include "debug.h"
#include "dispatcher.h"
#include "timerUtils.h"

volatile uint64_t red_last_us, green_last_us, yellow_last_us;

K_SEM_DEFINE(red_sem, 0, 1);
K_SEM_DEFINE(green_sem, 0, 1);
K_SEM_DEFINE(yellow_sem, 0, 1);
K_SEM_DEFINE(release_sem, 1, 1);

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// Thread function prototypes
static void red_task(void *, void *, void *);
static void green_task(void *, void *, void *);
static void yellow_task(void *, void *, void *);

// Initialize leds
int init_led() {
    int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_INACTIVE);
	if (ret) return ret;
	ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_INACTIVE);
	if (ret) return ret;
	ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_INACTIVE);
	if (ret) return ret;
    DBG("Led initialization ok\n");
    return 0;
}

static void set_leds(int r, int g, int b) {
	gpio_pin_set_dt(&red, r);
	gpio_pin_set_dt(&green, g);
	gpio_pin_set_dt(&blue, b);
}

static void red_task(void *p1, void *p2, void *p3) {
    while (1) {
        k_sem_take(&red_sem, K_FOREVER);

        uint64_t t0 = nowCyc();
        set_leds(1, 0, 0);
        DBG("Red on\n");
        uint64_t t1 = nowCyc();

        uint64_t cycles = t1 - t0;
        uint64_t ns     = cycToNs(cycles);
        uint64_t us     = cycToUs(cycles);

        red_last_us = us;
        DBG("Red task execution time: %llu us\n",
               (unsigned long long)red_last_us);

        uint64_t us_int = ns / 1000;
        unsigned int us_dec = (unsigned int)(ns % 1000);
        DBG("Red task time: %llu ns (%llu.%03u us), cycles=%llu\n",
            (unsigned long long)ns,
            (unsigned long long)us_int, us_dec,
            (unsigned long long)cycles);

        k_sleep(K_SECONDS(1));
        set_leds(0, 0, 0);
        DBG("Red off\n");

        on_task_done('r');
        k_sem_give(&release_sem);
    }
}


static void green_task(void *p1, void *p2, void *p3) {
    while (1) {
        k_sem_take(&green_sem, K_FOREVER);

        uint64_t t0 = nowCyc();
        set_leds(0, 1, 0);
        DBG("Green on\n");
        uint64_t t1 = nowCyc();

        uint64_t cycles = t1 - t0;
        uint64_t ns = cycToNs(cycles);
        uint64_t us = cycToUs(cycles);

        green_last_us = us;
        DBG("Green task execution time: %llu us\n",
               (unsigned long long)green_last_us);

        uint64_t us_int = ns / 1000;
        unsigned int us_dec = (unsigned int)(ns % 1000);
        DBG("Green task time: %llu ns (%llu.%03u us), cycles=%llu\n",
            (unsigned long long)ns,
            (unsigned long long)us_int, us_dec,
            (unsigned long long)cycles);

        k_sleep(K_SECONDS(1));
        set_leds(0, 0, 0);
        DBG("Green off\n");

        on_task_done('g');
        k_sem_give(&release_sem);
    }
}

static void yellow_task(void *p1, void *p2, void *p3) {
    while (1) {
        k_sem_take(&yellow_sem, K_FOREVER);

        uint64_t t0 = nowCyc();
        set_leds(1, 1, 0);
        DBG("Yellow on\n");
        uint64_t t1 = nowCyc();

        uint64_t cycles = t1 - t0;
        uint64_t ns     = cycToNs(cycles);
        uint64_t us     = cycToUs(cycles);

        yellow_last_us = us;
        DBG("Yellow task execution time: %llu us\n",
               (unsigned long long)yellow_last_us);

        uint64_t us_int = ns / 1000;
        unsigned int us_dec = (unsigned int)(ns % 1000);
        DBG("Yellow task time: %llu ns (%llu.%03u us), cycles=%llu\n",
            (unsigned long long)ns,
            (unsigned long long)us_int, us_dec,
            (unsigned long long)cycles);

        k_sleep(K_SECONDS(1));
        set_leds(0, 0, 0);
        DBG("Yellow off\n");

        on_task_done('y');
        k_sem_give(&release_sem);
    }
}


K_THREAD_DEFINE(red_thread, 1024, red_task, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(green_thread, 1024, green_task, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(yellow_thread, 1024, yellow_task, NULL, NULL, NULL, 5, 0, 0);
