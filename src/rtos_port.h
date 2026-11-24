#ifndef RTOS_PORT_H
#define RTOS_PORT_H

#include <stdint.h>

void rtos_tick_handler(void);
uint32_t *rtos_switch_context(uint32_t *saved_sp);

#endif
