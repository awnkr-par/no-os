#include <noos/irq.h>
#include <noos/memory.h>

struct irq_slot {
    noos_irq_handler handler;
    void *context;
};

static struct irq_slot g_slots[NOOS_MAX_IRQS];
static struct irq_stats g_stats;

void irq_init(void)
{
    memset(g_slots, 0, sizeof(g_slots));
    memset(&g_stats, 0, sizeof(g_stats));
    g_stats.last_irq = UINT32_MAX;
}

bool irq_register(uint32_t irq, noos_irq_handler handler, void *context)
{
    if (irq >= NOOS_MAX_IRQS || handler == 0 || g_slots[irq].handler != 0) {
        return false;
    }

    g_slots[irq].handler = handler;
    g_slots[irq].context = context;
    g_stats.registered++;
    return true;
}

bool irq_unregister(uint32_t irq)
{
    if (irq >= NOOS_MAX_IRQS || g_slots[irq].handler == 0) {
        return false;
    }

    g_slots[irq].handler = 0;
    g_slots[irq].context = 0;
    if (g_stats.registered != 0) {
        g_stats.registered--;
    }
    return true;
}

bool irq_dispatch(uint32_t irq)
{
    g_stats.dispatches++;
    g_stats.last_irq = irq;

    if (irq >= NOOS_MAX_IRQS || g_slots[irq].handler == 0) {
        g_stats.spurious++;
        return false;
    }

    g_slots[irq].handler(irq, g_slots[irq].context);
    g_stats.handled++;
    return true;
}

const struct irq_stats *irq_get_stats(void)
{
    return &g_stats;
}
