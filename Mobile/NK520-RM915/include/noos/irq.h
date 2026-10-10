#ifndef NOOS_IRQ_H
#define NOOS_IRQ_H

#include <stdbool.h>
#include <stdint.h>

#define NOOS_MAX_IRQS 256u

typedef void (*noos_irq_handler)(uint32_t irq, void *context);

struct irq_stats {
    uint32_t registered;
    uint32_t dispatches;
    uint32_t handled;
    uint32_t spurious;
    uint32_t last_irq;
};

void irq_init(void);
bool irq_register(uint32_t irq, noos_irq_handler handler, void *context);
bool irq_unregister(uint32_t irq);
bool irq_dispatch(uint32_t irq);
const struct irq_stats *irq_get_stats(void);

#endif
