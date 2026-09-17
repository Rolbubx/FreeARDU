.syntax unified
.cpu cortex-m7
.thumb

.macro FORWARD_FAULT name, target
    .global \name
    .section .text.\name, "ax", %progbits
    .align 2
    .type \name, %function
\name:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b \target
    .size \name, . - \name
.endm

FORWARD_FAULT MemManage_Handler, memmanage_handler_c
FORWARD_FAULT BusFault_Handler, bus_fault_handler_c
FORWARD_FAULT UsageFault_Handler, usage_fault_handler_c
