#ifndef NOOS_QGIC_H
#define NOOS_QGIC_H

#include <stdbool.h>
#include <stdint.h>

#define NOOS_QGIC_SPURIOUS_IRQ 1023u

struct qgic_stats {
    uint32_t initialized;
    uint32_t irq_count;
    uint32_t last_ack;
    uint32_t spurious;
    uint32_t unhandled;
};

bool qgic_init(void);
bool qgic_enable_irq(uint32_t irq, bool edge_rising);
void qgic_disable_irq(uint32_t irq);
bool qgic_handle_one(void);
const struct qgic_stats *qgic_get_stats(void);

#endif
