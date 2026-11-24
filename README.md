# MiniRTOS

MiniRTOS is a minimal educational kernel for the STM32 NUCLEO-F401RE (ARM Cortex-M4). It demonstrates **priority-based preemptive multitasking** with up to two user tasks, each with its own static stack. An idle task waits for interrupts when no user task is ready.

## Build and flash

Install GNU Make and the Arm GNU Toolchain (`arm-none-eabi-gcc`). Run from the repository root on Windows:

```sh
mingw32-make -j4
```

If the toolchain is installed somewhere other than the default in `Makefile`, pass `GCC_PATH="C:/path/to/toolchain/bin"` to the same command. Flash `build/minirtos.elf` using the board's built-in ST-LINK, then reset the board.

**Demo:** two external LEDs toggle independently: Arduino `D12` (`PA6`) every 1100 ms and `D11` (`PA7`) every 500 ms. Connect each output to an LED through its own series resistor, with the LED cathode going to `GND`. Both tasks sleep between toggles, so idle runs in between. See [the board test notes](docs/board-tests.md) for wiring and debugging.

## How it works

1. `main()` initializes HAL, the clock, and GPIO, then registers both tasks and starts the kernel.
2. SVC starts the highest-priority ready task on its own process stack (`PSP`).
3. The shared 1 ms SysTick updates the HAL tick, counts down sleeping tasks, and requests PendSV. PendSV switches to the highest-priority ready task. Equal priorities rotate round-robin. If all tasks sleep, idle executes `WFI`.

## Public API

Register one or two tasks before calling `rtos_start()`. A larger `priority` value means higher priority. Call `task_delay()` from a task with interrupts enabled; a zero delay does nothing. Each task needs a separate, persistent, 8-byte-aligned stack. `size` is measured in `uint32_t` words, not bytes. Task entry functions must not return.

## Project layout

- [`Core/Src/main.c`](Core/Src/main.c) — board initialization and the two demo tasks.
- [`Core/Src/stm32f4xx_it.c`](Core/Src/stm32f4xx_it.c) — SVC, PendSV, and SysTick entry points.
- [`src/task.c`](src/task.c) and [`src/task.h`](src/task.h) — task registration and scheduler.
- [`src/rtos_context.s`](src/rtos_context.s) — Cortex-M4 context save and restore.
- `Drivers/` — STM32 HAL and CMSIS.
