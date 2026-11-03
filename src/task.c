#include "task.h"
// #include "stm32f4xx.h"
#include <stddef.h>

#define MAX_TASK_COUNT 4U

static task_handler_t task_handlers[MAX_TASK_COUNT];
static uint32_t task_count;

static void idle_task(void *param) {
  (void)param;

  for (;;) {
    // __WFI();
  }
}

static void task_exit() {
  while (1)
    ;
}

task_handler_t *task_create(void (*task)(void *), uint32_t *stack,
                            uint32_t size, uint8_t priority, void *param) {
  if (task_count >= MAX_TASK_COUNT) {
    return NULL;
  }

  task_handler_t *handler = &task_handlers[task_count];
  ++task_count;
  static task_handler_t *task_now, *task_head, *task_last;

  /* TCB и стек */
  stack[size - 1] = 0x01000000;
  stack[size - 2] = (uint32_t)task;
  stack[size - 3] = (uint32_t)task_exit;
  stack[size - 4] = 0x00000000;
  stack[size - 5] = 0x00000000;
  stack[size - 6] = 0x00000000;
  stack[size - 7] = 0x00000000;
  stack[size - 8] = (uint32_t)param;

  handler->PSP = stack + size - 16;
  handler->state = ready;
  handler->priority = priority;
  handler->timeout = 0;
  handler->next = handler;
  task_last->next = task_head;

  if (task == NULL || stack == NULL || priority == 0) {
    return NULL;
  }

  if (task_head == NULL) {
    task_head = handler;
    task_last = handler;
    handler->next = handler;
  } else {
    handler->next = task_head;
    task_last->next = handler;
    task_last = handler;
  }

  return handler;
}
