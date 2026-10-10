#ifndef NOOS_TIMER_H
#define NOOS_TIMER_H

#include <stdbool.h>
#include <stdint.h>

#define NOOS_RM915_GIC_DIST_BASE 0x02000000u
#define NOOS_RM915_GIC_CPU_BASE  0x02002000u
#define NOOS_RM915_TMR_BASE      0x0200A000u

struct rm915_timer_plan {
    uint32_t gic_dist_base;
    uint32_t gic_cpu_base;

    uint32_t gpt_count;
    uint32_t gpt_enable;
    uint32_t gpt_clear;

    uint32_t dgt_count;
    uint32_t dgt_enable;
    uint32_t dgt_clear;
    uint32_t dgt_clk_ctl;
    uint32_t dgt_clk_divider_value;
    uint32_t dgt_ticks_per_sec;
};

const struct rm915_timer_plan *rm915_timer_plan(void);
bool rm915_timer_validate_plan(void);

#endif
