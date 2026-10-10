#ifndef NOOS_CPU_H
#define NOOS_CPU_H

#include <stdint.h>

struct cpu_state {
    uint32_t midr;
    uint32_t cpsr;
    uint32_t sctlr;
};

void cpu_read_state(struct cpu_state *state);

static inline bool cpu_mmu_enabled(const struct cpu_state *state)
{
    return (state->sctlr & 1u) != 0;
}

static inline bool cpu_dcache_enabled(const struct cpu_state *state)
{
    return (state->sctlr & (1u << 2)) != 0;
}

static inline bool cpu_icache_enabled(const struct cpu_state *state)
{
    return (state->sctlr & (1u << 12)) != 0;
}

#endif
