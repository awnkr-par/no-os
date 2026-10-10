#include <noos/timer.h>

#define RM915_GPT_BASE       (NOOS_RM915_TMR_BASE + 0x04u)
#define RM915_DGT_BASE       (NOOS_RM915_TMR_BASE + 0x24u)

#define RM915_GPT_COUNT      (RM915_GPT_BASE + 0x04u)
#define RM915_GPT_ENABLE     (RM915_GPT_BASE + 0x08u)
#define RM915_GPT_CLEAR      (RM915_GPT_BASE + 0x0Cu)

#define RM915_DGT_COUNT      (RM915_DGT_BASE + 0x04u)
#define RM915_DGT_ENABLE     (RM915_DGT_BASE + 0x08u)
#define RM915_DGT_CLEAR      (RM915_DGT_BASE + 0x0Cu)
#define RM915_DGT_CLK_CTL    (RM915_DGT_BASE + 0x10u)

#define RM915_DGT_DIV_VALUE  3u
#define RM915_DGT_HZ         6750000u

_Static_assert(RM915_GPT_COUNT == 0x0200A008u,
               "RM915 GPT count address mismatch");
_Static_assert(RM915_GPT_ENABLE == 0x0200A00Cu,
               "RM915 GPT enable address mismatch");
_Static_assert(RM915_GPT_CLEAR == 0x0200A010u,
               "RM915 GPT clear address mismatch");
_Static_assert(RM915_DGT_COUNT == 0x0200A028u,
               "RM915 DGT count address mismatch");
_Static_assert(RM915_DGT_ENABLE == 0x0200A02Cu,
               "RM915 DGT enable address mismatch");
_Static_assert(RM915_DGT_CLEAR == 0x0200A030u,
               "RM915 DGT clear address mismatch");
_Static_assert(RM915_DGT_CLK_CTL == 0x0200A034u,
               "RM915 DGT clock-control address mismatch");

static const struct rm915_timer_plan g_plan = {
    .gic_dist_base = NOOS_RM915_GIC_DIST_BASE,
    .gic_cpu_base = NOOS_RM915_GIC_CPU_BASE,

    .gpt_count = RM915_GPT_COUNT,
    .gpt_enable = RM915_GPT_ENABLE,
    .gpt_clear = RM915_GPT_CLEAR,

    .dgt_count = RM915_DGT_COUNT,
    .dgt_enable = RM915_DGT_ENABLE,
    .dgt_clear = RM915_DGT_CLEAR,
    .dgt_clk_ctl = RM915_DGT_CLK_CTL,
    .dgt_clk_divider_value = RM915_DGT_DIV_VALUE,
    .dgt_ticks_per_sec = RM915_DGT_HZ,
};

const struct rm915_timer_plan *rm915_timer_plan(void)
{
    return &g_plan;
}

bool rm915_timer_validate_plan(void)
{
    return
        g_plan.gic_dist_base == 0x02000000u &&
        g_plan.gic_cpu_base == 0x02002000u &&
        g_plan.gpt_count == 0x0200A008u &&
        g_plan.gpt_enable == 0x0200A00Cu &&
        g_plan.gpt_clear == 0x0200A010u &&
        g_plan.dgt_count == 0x0200A028u &&
        g_plan.dgt_enable == 0x0200A02Cu &&
        g_plan.dgt_clear == 0x0200A030u &&
        g_plan.dgt_clk_ctl == 0x0200A034u &&
        g_plan.dgt_clk_divider_value == 3u &&
        g_plan.dgt_ticks_per_sec == 6750000u;
}
