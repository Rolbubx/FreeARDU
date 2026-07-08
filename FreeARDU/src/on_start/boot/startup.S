.syntax unified
.cpu cortex-m7
.fpu fpv5-d16
.thumb

.global reset_handler
.global Reset_Handler
.global vector_table

/* Make reset_handler weak so it doesn't conflict with crt0.o's _start */
.weak reset_handler
.weak Reset_Handler

.extern _estack
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss
.extern __init_array_start
.extern __init_array_end

.weak SystemInit
.weak main

/*
 * C++ symbols for:
 * void setup()
 * void loop()
 */
.weak _Z5setupv
.weak _Z4loopv

.section .isr_vector, "a", %progbits
.align 7

vector_table:
    .word _estack
    .word reset_handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

.section .text.reset_handler, "ax", %progbits
.align 2
.type reset_handler, %function

reset_handler:
Reset_Handler:
    cpsid i

    /*
     * Set stack pointer.
     */
    ldr r0, =_estack
    mov sp, r0

    /*
     * Copy .data from flash to RAM.
     *
     * r0 = source flash address
     * r1 = destination RAM start
     * r2 = destination RAM end
     */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

.L_copy_data:
    cmp r1, r2
    bhs .L_zero_bss_start
    ldr r3, [r0], #4
    str r3, [r1], #4
    b .L_copy_data

.L_zero_bss_start:
    /*
     * Clear .bss.
     *
     * r0 = RAM start
     * r1 = RAM end
     * r2 = zero
     */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0

.L_zero_bss:
    cmp r0, r1
    bhs .L_system_init
    str r2, [r0], #4
    b .L_zero_bss

.L_system_init:
    /*
     * Optional chip/clock/pin low-level init.
     * If SystemInit is not provided, the weak default below returns.
     */
    ldr r0, =SystemInit
    cbz r0, .L_cpp_init
    bl SystemInit

.L_cpp_init:
    /*
     * Run C++ global constructors.
     * Needed for global objects like framebuffer.
     */
    ldr r0, =__init_array_start
    ldr r1, =__init_array_end

.L_init_loop:
    cmp r0, r1
    bhs .L_enable_irq
    ldr r2, [r0], #4
    blx r2
    b .L_init_loop

.L_enable_irq:
    cpsie i

.L_enter_app:
    /*
     * If a normal C/C++ main() exists, use it.
     */
    ldr r0, =main
    cbz r0, .L_try_arduino_style
    bl main
    b .L_hang

.L_try_arduino_style:
    /*
     * Otherwise support Arduino-style:
     *
     * void setup();
     * void loop();
     */
    ldr r0, =_Z5setupv
    cbz r0, .L_loop_forever
    bl _Z5setupv

.L_loop_forever:
    ldr r0, =_Z4loopv
    cbz r0, .L_hang
    bl _Z4loopv
    b .L_loop_forever

.L_hang:
    b .L_hang

.size reset_handler, . - reset_handler

.section .text.default_handlers, "ax", %progbits
.align 2

.type Default_Handler, %function
Default_Handler:
    b Default_Handler
.size Default_Handler, . - Default_Handler

.type Default_SystemInit, %function
Default_SystemInit:
    bx lr
.size Default_SystemInit, . - Default_SystemInit

.weak NMI_Handler
.thumb_set NMI_Handler, Default_Handler

.weak HardFault_Handler
.thumb_set HardFault_Handler, Default_Handler

.weak MemManage_Handler
.thumb_set MemManage_Handler, Default_Handler

.weak BusFault_Handler
.thumb_set BusFault_Handler, Default_Handler

.weak UsageFault_Handler
.thumb_set UsageFault_Handler, Default_Handler

.weak SVC_Handler
.thumb_set SVC_Handler, Default_Handler

.weak DebugMon_Handler
.thumb_set DebugMon_Handler, Default_Handler

.weak PendSV_Handler
.thumb_set PendSV_Handler, Default_Handler

.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler

.thumb_set SystemInit, Default_SystemInit