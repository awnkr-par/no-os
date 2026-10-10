#include <noos/irq.h>
#include <noos/mmio.h>
#include <noos/qgic.h>
#include <noos/timer.h>

#define GIC_CPU_CTRL            0x000u
#define GIC_CPU_PRIMASK         0x004u
#define GIC_CPU_INTACK          0x00Cu
#define GIC_CPU_EOI             0x010u

#define GIC_DIST_CTRL           0x000u
#define GIC_DIST_CTR            0x004u
#define GIC_DIST_ENABLE_SET     0x100u
#define GIC_DIST_ENABLE_CLEAR   0x180u
#define GIC_DIST_PENDING_CLEAR  0x280u
#define GIC_DIST_PRI            0x400u
#define GIC_DIST_CONFIG         0xC00u

static struct qgic_stats g_stats;
static uint32_t g_irq_lines;

static void barrier(void)
{
    __asm__ volatile ("dsb sy\n\tisb" ::: "memory");
}

bool qgic_init(void)
{
    g_stats.initialized = 0;
    g_stats.irq_count = 0;
    g_stats.last_ack = NOOS_QGIC_SPURIOUS_IRQ;
    g_stats.spurious = 0;
    g_stats.unhandled = 0;

    const uint32_t typer = mmio_read32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_CTR);
    g_irq_lines = ((typer & 0x1Fu) + 1u) * 32u;
    if (g_irq_lines < 32u || g_irq_lines > 1024u) {
        return false;
    }

    mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_CTRL, 0u);

    for (uint32_t irq = 32u; irq < g_irq_lines; irq += 32u) {
        mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_ENABLE_CLEAR +
                     (irq / 32u) * 4u, 0xFFFFFFFFu);
        mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_PENDING_CLEAR +
                     (irq / 32u) * 4u, 0xFFFFFFFFu);
    }

    mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_ENABLE_CLEAR, 0xFFFF0000u);
    mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_ENABLE_SET, 0x0000FFFFu);

    for (uint32_t irq = 0; irq < 32u; irq += 4u) {
        mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_PRI + irq,
                     0xA0A0A0A0u);
    }

    mmio_write32(NOOS_RM915_GIC_CPU_BASE + GIC_CPU_PRIMASK, 0xF0u);
    mmio_write32(NOOS_RM915_GIC_CPU_BASE + GIC_CPU_CTRL, 1u);
    mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_CTRL, 1u);
    barrier();

    g_stats.initialized = 1u;
    return true;
}

bool qgic_enable_irq(uint32_t irq, bool edge_rising)
{
    if (!g_stats.initialized || irq >= g_irq_lines || irq < 16u) {
        return false;
    }

    const uint32_t bit = 1u << (irq & 31u);
    const uintptr_t enable_clear = NOOS_RM915_GIC_DIST_BASE +
        GIC_DIST_ENABLE_CLEAR + (irq / 32u) * 4u;
    const uintptr_t enable_set = NOOS_RM915_GIC_DIST_BASE +
        GIC_DIST_ENABLE_SET + (irq / 32u) * 4u;

    mmio_write32(enable_clear, bit);

    const uintptr_t config_reg = NOOS_RM915_GIC_DIST_BASE +
        GIC_DIST_CONFIG + (irq / 16u) * 4u;
    const uint32_t shift = (irq & 15u) * 2u;
    uint32_t config = mmio_read32(config_reg);
    config &= ~(3u << shift);
    if (edge_rising) {
        config |= 2u << shift;
    }
    mmio_write32(config_reg, config);

    const uintptr_t pri_reg = NOOS_RM915_GIC_DIST_BASE +
        GIC_DIST_PRI + (irq & ~3u);
    const uint32_t pri_shift = (irq & 3u) * 8u;
    uint32_t pri = mmio_read32(pri_reg);
    pri &= ~(0xFFu << pri_shift);
    pri |= 0xA0u << pri_shift;
    mmio_write32(pri_reg, pri);

    mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_PENDING_CLEAR +
                 (irq / 32u) * 4u, bit);
    mmio_write32(enable_set, bit);
    barrier();
    return true;
}

void qgic_disable_irq(uint32_t irq)
{
    if (!g_stats.initialized || irq >= g_irq_lines) {
        return;
    }

    mmio_write32(NOOS_RM915_GIC_DIST_BASE + GIC_DIST_ENABLE_CLEAR +
                 (irq / 32u) * 4u, 1u << (irq & 31u));
    barrier();
}

bool qgic_handle_one(void)
{
    if (!g_stats.initialized) {
        return false;
    }

    const uint32_t ack = mmio_read32(NOOS_RM915_GIC_CPU_BASE + GIC_CPU_INTACK);
    const uint32_t irq = ack & 0x3FFu;
    g_stats.last_ack = ack;

    if (irq >= 1020u) {
        g_stats.spurious++;
        return true;
    }

    g_stats.irq_count++;
    const bool handled = irq_dispatch(irq);
    if (!handled) {
        g_stats.unhandled++;
    }

    mmio_write32(NOOS_RM915_GIC_CPU_BASE + GIC_CPU_EOI, ack);
    barrier();
    return handled;
}

const struct qgic_stats *qgic_get_stats(void)
{
    return &g_stats;
}
