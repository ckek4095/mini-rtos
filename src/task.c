#include "task.h"
#include "rtos_port.h"
#include "stm32f4xx.h"
#include <stddef.h>

#define TASK_COUNT 2U
#define CONTEXT_WORDS 16U

typedef struct {
  uint32_t *sp;
  uint32_t sleep_ticks;
  uint8_t priority;
} task_t;

static task_t tasks[TASK_COUNT], idle;
static _Alignas(8) uint32_t idle_stack[64];
static uint32_t registered, cursor;
static task_t *current;

static void idle_task(void) {
  for (;;) {
    __WFI();
  }
}

static uint32_t *init_stack(void (*entry)(void), uint32_t *top) {
  uint32_t *sp = top - CONTEXT_WORDS;
  for (uint32_t i = 0; i < CONTEXT_WORDS; ++i) {
    sp[i] = 0U;
  }
  /* R4–R11 precede the hardware exception frame. */
  sp[13] = (uint32_t)(uintptr_t)idle_task;   /* LR: fallback if entry returns */
  sp[14] = (uint32_t)(uintptr_t)entry & ~1U; /* PC */
  sp[15] = 1UL << 24;                        /* xPSR.T */
  return sp;
}

bool task_create(void (*entry)(void), uint32_t *stack, uint32_t size,
                 uint8_t priority) {
  if (current != NULL || registered == TASK_COUNT || entry == NULL ||
      stack == NULL || size < CONTEXT_WORDS || (size & 1U) != 0U ||
      ((uintptr_t)stack & 7U) != 0U) {
    return false;
  }

  tasks[registered++] =
      (task_t){.sp = init_stack(entry, stack + size), .priority = priority};
  return true;
}

uint32_t *rtos_switch_context(uint32_t *saved_sp) {
  if (saved_sp != NULL) {
    current->sp = saved_sp;
  }

  task_t *next = &idle;
  uint32_t selected = cursor;
  for (uint32_t offset = 0; offset < registered; ++offset) {
    uint32_t i = (cursor + offset) % registered;
    if (tasks[i].sleep_ticks == 0U &&
        (next == &idle || tasks[i].priority > next->priority)) {
      next = &tasks[i];
      selected = i;
    }
  }
  if (next != &idle) {
    cursor = (selected + 1U) % registered;
  }
  current = next;
  return current->sp;
}

void rtos_start(void) {
  if (current != NULL || registered == 0U) {
    return;
  }

  idle.sp = init_stack(idle_task, idle_stack + 64U);
  const uint32_t lowest = (1UL << __NVIC_PRIO_BITS) - 1U;
  SCB->CCR |= SCB_CCR_STKALIGN_Msk;
  NVIC_SetPriority(SVCall_IRQn, 0U);
  NVIC_SetPriority(SysTick_IRQn, lowest - 1U);
  NVIC_SetPriority(PendSV_IRQn, lowest);
  __asm volatile("svc #0" ::: "memory");
}

void task_delay(uint32_t ms) {
  if (ms == 0U || __get_IPSR() != 0U || current == NULL || current == &idle ||
      __get_PRIMASK() != 0U || __get_BASEPRI() != 0U ||
      __get_FAULTMASK() != 0U) {
    return;
  }

  __disable_irq();
  current->sleep_ticks = ms;
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
  __DSB();
  __enable_irq();
  __ISB();
}

void rtos_tick_handler(void) {
  if (current == NULL) {
    return;
  }
  for (uint32_t i = 0; i < registered; ++i) {
    if (tasks[i].sleep_ticks != 0U) {
      --tasks[i].sleep_ticks;
    }
  }
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}
