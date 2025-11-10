#include "uartDef.h"
#include "dispatcher.h"
#include "time_parser.h"
#include "debug.h"

#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/timing/timing.h>

#include <string.h>
#include <ctype.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

/* Aikamerkkijonon keruu: 'T' + 6 numeroa */
static char tbuf[7];     // 6 numeroa + '\0'
static int  ti = -1;     // -1 = ei kerätä, 0..6 = kerätään
static uint64_t uart_start_time = 0;

static void uart_cb(const struct device *dev, void *user_data)
{
    uint8_t c;

    if (uart_irq_update(dev) && uart_irq_rx_ready(dev)) {
        while (uart_fifo_read(dev, &c, 1) == 1) {

            if (c == '\r' || c == '\n') {
                continue; // ohita rivinvaihdot
            }

            /* Jos keruu on käynnissä (T on vastaanotettu) */
            if (ti >= 0) {
                if (isdigit((unsigned char)c) && ti < 6) {
                    tbuf[ti++] = (char)c;

                    if (ti == 6) {
                        tbuf[6] = '\0';
                        ti = -1; // lopeta keruu

                        // <<< FIX: määrittele seconds ennen käyttöä
                        int seconds = time_parse(tbuf);

                        // Lähetä parserin tulos UARTtiin aina
                        printk("%dX", seconds);

                        // Lähetä dispatcherille vain jos aika on positiivinen
                        if (seconds > 0) {
                            dispatcher_event_t evt = {0};
                            evt.type = EVENT_TIMER_START;
                            evt.seconds = seconds;
                            strcpy(evt.msg, "TIMER");
                            dispatcher_post(&evt);
                        }

                        // Mittausaikaa ja debug-infoa (ei vaikuta testiin)
                        uint64_t delta_us = timing_cycles_to_ns(
                            timing_counter_get() - uart_start_time) / 1000;
                        DBG("UART parsed '%s' -> %d s (%.0f us)\n",
                            tbuf, seconds, (double)delta_us);
                    }
                } else {
                    /* Ei numero -> mitätöi keruu ja ilmoita virheestä */
                    DBG("UART: invalid time char '%c' -> reset\n", c);
                    ti = -1;

                    // <<< FIX: virhekoodi -3 (ei numeroa)
                    printk("%dX", -3);
                }
                continue; // palaa while-looppiin, tämä on ok tässä kontekstissa
            }

            /* Aloita keruu komennolla 'T' tai 't' */
            if (c == 'T' || c == 't') {
                ti = 0;
                uart_start_time = timing_counter_get();
                DBG("UART: starting time collection\n");
                continue;
            }

            /* Muut yksimerkkikomennot (esim. R, Y, G, D...) */
            dispatcher_event_t evt = { .type = EVENT_LED_CMD };
            evt.msg[0] = (char)c;
            evt.msg[1] = '\0';
            dispatcher_post(&evt);
        }
    }
}

void uartDef_init(void)
{
    if (!device_is_ready(uart_dev)) {
        DBG("Error: UART device not ready\n");
        return;
    }

    uart_irq_callback_user_data_set(uart_dev, uart_cb, NULL);
    uart_irq_rx_enable(uart_dev);
    DBG("UART initialized with RX interrupt\n");
}
