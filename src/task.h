#ifndef TASK_H
#define TASK_H

#include <stdbool.h>
#include <stdint.h>

/* Register before start. Higher numeric priority runs first. */
bool task_create(void (*entry)(void), uint32_t *stack, uint32_t size,
                 uint8_t priority);
void rtos_start(void);
/* Call from a user task with interrupts enabled; zero is a no-op. */
void task_delay(uint32_t ms);

#endif
