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
    mov r4, r0
    mov r5, r1
    mov r6, r2

    cpsid if

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

    ldr r0, =_start
    mcr p15, 0, r0, c12, c0, 0
    dsb
    isb

    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
1:
    cmp r0, r1
    strlo r2, [r0], #4
    blo 1b

    mov r0, r4
    mov r1, r5
    mov r2, r6
    bl kernel_main

2:
    wfi
    b 2b

.align 2
vector_undefined:
    stmdb sp!, {r0-r12, lr}
    mrs r1, spsr
    mov r2, sp
    mov r0, #1
    bl exception_handle
    b exception_dead

.align 2
vector_svc:
    stmdb sp!, {r0-r12, lr}
    mrs r1, spsr
    mov r2, sp
    mov r0, #2
    bl exception_handle
    b exception_dead

.align 2
vector_prefetch_abort:
    stmdb sp!, {r0-r12, lr}
    mrs r1, spsr
    mov r2, sp
    mov r0, #3
    bl exception_handle
    b exception_dead

.align 2
vector_data_abort:
    stmdb sp!, {r0-r12, lr}
    mrs r1, spsr
    mov r2, sp
    mov r0, #4
    bl exception_handle
    b exception_dead

.align 2
vector_reserved:
    stmdb sp!, {r0-r12, lr}
    mrs r1, spsr
    mov r2, sp
    mov r0, #5
    bl exception_handle
    b exception_dead

.align 2
vector_irq:
    stmdb sp!, {r0-r12, lr}
    mrs r1, spsr
    mov r2, sp
    mov r0, #6
    bl exception_handle
    b exception_dead

.align 2
vector_fiq:
    stmdb sp!, {r0-r12, lr}
    mrs r1, spsr
    mov r2, sp
    mov r0, #7
    bl exception_handle

exception_dead:
    wfi
    b exception_dead
