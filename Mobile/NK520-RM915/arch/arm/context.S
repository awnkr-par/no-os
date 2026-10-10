.syntax unified
.arch armv7-a
.arm

.section .text.context_switch, "ax", %progbits
.align 2

.global arm_context_switch
.type arm_context_switch, %function
arm_context_switch:
    stmia r0, {r4-r11, sp, lr}
    ldmia r1, {r4-r11, sp, lr}
    bx lr
.size arm_context_switch, . - arm_context_switch
