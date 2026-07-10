.syntax unified
.cpu cortex-m7
.thumb

.global HardFault_Handler

.section .text.HardFault_Handler, "ax", %progbits
.align 2
.type HardFault_Handler, %function
HardFault_Handler:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b hard_fault_handler_c
.size HardFault_Handler, . - HardFault_Handler