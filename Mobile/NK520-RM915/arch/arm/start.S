.syntax unified
.arch armv7-a
.arm

.section .vectors, "ax", %progbits
.align 5
.global _start
.type _start, %function
_start:
    b reset_handler
    b exception_hang
    b exception_hang
    b exception_hang
    b exception_hang
    b exception_hang
    b exception_hang
    b exception_hang
.size _start, . - _start

.section .text.boot, "ax", %progbits
.align 2
.type reset_handler, %function
reset_handler:
    /* Preserve the standard ARM32 bootloader handoff registers. */
    mov r4, r0
    mov r5, r1
    mov r6, r2

    cpsid if

    /* Install our vector table at the address where the image was loaded. */
    ldr r0, =_start
    mcr p15, 0, r0, c12, c0, 0
    isb

    ldr sp, =__stack_top

    /* A freestanding C runtime still requires zero-initialized .bss. */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
1:
    cmp r0, r1
    strlo r2, [r0], #4
    blo 1b

    /* kernel_main(boot_r0, machine_type, atags_or_fdt). */
    mov r0, r4
    mov r1, r5
    mov r2, r6
    bl kernel_main

exception_hang:
    wfi
    b exception_hang
