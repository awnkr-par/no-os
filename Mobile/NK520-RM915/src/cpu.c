#include <noos/cpu.h>

void cpu_read_state(struct cpu_state *state)
{
    if (state == 0) {
        return;
    }

    __asm__ volatile ("mrc p15, 0, %0, c0, c0, 0" : "=r"(state->midr));
    __asm__ volatile ("mrs %0, cpsr" : "=r"(state->cpsr));
    __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(state->sctlr));
}
