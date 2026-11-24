.syntax unified
.cpu cortex-m4
.thumb

.section .text.rtos_port_start,"ax",%progbits
.global rtos_port_start
.type rtos_port_start,%function
.thumb_func
rtos_port_start:
    cpsid i
    movs r0, #0
    bl rtos_switch_context
    ldmia r0!, {r4-r11}
    msr psp, r0

    mrs r1, msp
    ldr r2, [r1, #28]
    add r1, r1, #32
    tst r2, #0x200
    it ne
    addne r1, r1, #4
    msr msp, r1

    ldr lr, =0xFFFFFFFD
    cpsie i
    bx lr
.size rtos_port_start, .-rtos_port_start

.section .text.rtos_port_switch,"ax",%progbits
.global rtos_port_switch
.type rtos_port_switch,%function
.thumb_func
rtos_port_switch:
    cpsid i
    mrs r0, psp
    stmdb r0!, {r4-r11}
    push {r3, lr}
    bl rtos_switch_context
    pop {r3, lr}
    ldmia r0!, {r4-r11}
    msr psp, r0
    cpsie i
    bx lr
.size rtos_port_switch, .-rtos_port_switch

.section .note.GNU-stack,"",%progbits
