.syntax unified
.arch armv7-a
.arm

.section .vectors, "ax", %progbits
.align 5
.global _start
.type _start, %function
_start:
    b reset_handler
    b vector_undefined
    b vector_svc
    b vector_prefetch_abort
    b vector_data_abort
    b vector_reserved
    b vector_irq
    b vector_fiq
.size _start, . - _start

.section .text.boot, "ax", %progbits
.align 2
.type reset_handler, %function
reset_handler:
    /* Preserve the ARM32 bootloader handoff registers. */
    mov r4, r0
    mov r5, r1
    mov r6, r2

    cpsid if

    /* Give every exception mode its own banked stack. */
    cps #0x13
    ldr sp, =__svc_stack_top
    cps #0x12
    ldr sp, =__irq_stack_top
    cps #0x11
    ldr sp, =__fiq_stack_top
    cps #0x17
    ldr sp, =__abt_stack_top
    cps #0x1b
    ldr sp, =__und_stack_top
    cps #0x13
    ldr sp, =__svc_stack_top

    /* Install the noOS vector table. */
    ldr r0, =_start
    mcr p15, 0, r0, c12, c0, 0
    dsb
    isb

    /* A freestanding C runtime requires zero-initialized .bss. */
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

2:
    wfi
    b 2b

.macro FATAL_VECTOR name, type
.align 2

ame:
    stmdb sp!, {r0-r12, lr}
    mrs r1, spsr
    mov r2, sp
    mov r0, #	ype
    bl exception_handle
1:
    wfi
    b 1b
.endm

FATAL_VECTOR vector_undefined, 1
FATAL_VECTOR vector_svc, 2
FATAL_VECTOR vector_prefetch_abort, 3
FATAL_VECTOR vector_data_abort, 4
FATAL_VECTOR vector_reserved, 5
FATAL_VECTOR vector_irq, 6
FATAL_VECTOR vector_fiq, 7
