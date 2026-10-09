# MiniRTOS

MiniRTOS is a small educational real-time operating system kernel for ARM Cortex-M microcontrollers. Its goal is to show how tasks, separate stacks, timer interrupts, scheduling, and context switching work at a low level.

The project is not intended to replace a production RTOS such as FreeRTOS.

## Target Platform

- **Architecture:** ARM Cortex-M4
- **Recommended board:** STM32 NUCLEO-F401RE
- **Language:** C11 with a small amount of ARM Assembly
- **Compiler:** `arm-none-eabi-gcc`
- **Build system:** CMake
- **Core support:** CMSIS-Core
- **Debugging:** GDB with OpenOCD or ST-LINK

## Main Features

The required version of MiniRTOS will provide:

- statically created tasks;
- a separate fixed-size stack for each task;
- Task Control Blocks (TCBs);
- `READY`, `RUNNING`, and `SLEEPING` task states;
- cooperative switching with `task_yield()`;
- preemptive Round Robin scheduling;
- a 1 ms system tick using SysTick;
- context switching through PendSV;
- non-blocking task delays;
- an idle task that can put the CPU into sleep mode;
- basic runtime diagnostics.

Dynamic memory allocation is not used. The maximum task count and stack sizes are selected at compile time.

## Public API

The planned public API is intentionally small:

```c
rtos_init();
task_create();
rtos_start();
task_yield();
task_delay();
rtos_get_ticks();
```

Application code should use this API instead of accessing scheduler data or TCBs directly.

## How Scheduling Works

Ready tasks run one after another with Round Robin scheduling. SysTick updates the kernel time and requests a context switch. PendSV saves the current task context, selects the next ready task, and restores its context.

Sleeping tasks are skipped until their wake-up time is reached. The idle task runs when no user task is ready.

The Cortex-M port must correctly use PSP and MSP and handle the registers saved by hardware and software during exception entry and return.

## Project Structure

```text
include/    Public RTOS headers
kernel/     Tasks, scheduler, timer, and kernel state
port/       Cortex-M context switch and exception handlers
examples/   Demonstration applications
tests/      Scheduler and kernel tests
docs/       Architecture and implementation notes
```

Platform-independent kernel logic should remain separate from ARM-specific code.

## Demonstration

The main example will run at least three periodic tasks, such as:

- a task that toggles an LED every 100 ms;
- a task that toggles another LED every 500 ms;
- a task that prints a UART message every 1000 ms.

A separate preemption test will contain one task with an infinite calculation loop and no call to `task_yield()`. Another periodic task must continue to run, proving that timer-based preemption works.

## Validation Goals

The project should demonstrate:

- correct task creation and independent stacks;
- Round Robin order and voluntary yielding;
- working SysTick and tick counting;
- sleeping and waking without busy waiting;
- preemption of a task that never yields;
- idle-task execution when all user tasks sleep;
- stable long-running context switching.

## Out of Scope

The required version does not include priorities, mutexes, semaphores, queues, dynamic task management, MPU isolation, networking, filesystems, POSIX support, or hard real-time guarantees.

Possible future work includes CPU usage statistics, runtime task information, or simple stack-overflow detection.

## Development Plan

1. Create TCBs, task stacks, and a cooperative scheduler.
2. Implement PendSV context switching and Round Robin selection.
3. Add SysTick preemption, delays, task states, and the idle task.
4. Test the kernel and prepare examples and documentation.

The most important learning result is understanding what is stored on each task stack and how Cortex-M resumes a different task after an exception return.
