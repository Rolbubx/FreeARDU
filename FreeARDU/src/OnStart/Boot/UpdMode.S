.syntax unified
.cpu cortex-m7
.thumb

.global upd_mode_entry

.section .text.upd_mode_entry, "ax", %progbits
.align 2
.type upd_mode_entry, %function

@ upd_mode_entry(uint32_t mhz)
upd_mode_entry:
    push {r4, lr}
    mov r4, r0 @ Store the requested MHz

    @ In a real scenario, this would configure clocks.
    @ For this simulation, we'll just return it so C code can use it for delays.
    mov r0, r4
    pop {r4, pc}
