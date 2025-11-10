#ifndef DISPATCHER_H
#define DISPATCHER_H


#include <stdint.h>

typedef enum {
    EVENT_LED_CMD,
    EVENT_TIMER_START
} event_type_t;

typedef struct {
    void *fifo_reserved;
    event_type_t type;
    char msg[20];
    uint64_t t_cyc;
    uint8_t  src;
    int32_t  seconds;  
} dispatcher_event_t;

void dispatcher_post(dispatcher_event_t *evt);
void dispatcher_init(void);
void on_task_done(char which);

#endif
