#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include "dispatcher.h"
#include "debug.h"

// Configure buttons
#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
#define BUTTON_2 DT_ALIAS(sw2)

static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {0});
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {0});

static struct gpio_callback button_cb_data[3];

// Shared handler for all buttons
void button_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    char msg_char = 0;

    // Pienet kirjaimet vastaavat dispatcherin logiikkaa
    if (pins & BIT(button_0.pin)) msg_char = 'r';
    else if (pins & BIT(button_1.pin)) msg_char = 'y';
    else if (pins & BIT(button_2.pin)) msg_char = 'g';

    if (msg_char) {
        DBG("Button pressed -> sending '%c'\n", msg_char);
        dispatcher_event_t evt = {
            .type = EVENT_LED_CMD,
            .msg = {msg_char, '\0'}  // 1 merkki + null-terminaattori
        };
        dispatcher_post(&evt);
    }
}

// Initialize one button (helper)
static int setup_button(const struct gpio_dt_spec *button, struct gpio_callback *cb)
{
    if (!device_is_ready(button->port)) {
        DBG("Button port not ready\n");
        return -1;
    }

    int ret = gpio_pin_configure_dt(button, GPIO_INPUT);
    if (ret) return ret;

    ret = gpio_pin_interrupt_configure_dt(button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret) return ret;

    gpio_init_callback(cb, button_handler, BIT(button->pin));
    gpio_add_callback(button->port, cb);

    return 0;
}

// Initialize all buttons
int init_button() {
    int ret = 0;

    DBG("Initializing buttons...\n");

    ret |= setup_button(&button_0, &button_cb_data[0]);
    ret |= setup_button(&button_1, &button_cb_data[1]);
    ret |= setup_button(&button_2, &button_cb_data[2]);

    if (ret == 0)
        DBG("All buttons initialized successfully.\n");
    else
        DBG("One or more buttons failed to initialize.\n");

    return ret;
}
