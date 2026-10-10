#ifndef NOOS_EXCEPTION_H
#define NOOS_EXCEPTION_H

#include <stdint.h>

enum noos_exception_type {
    NOOS_EXC_UNDEFINED = 1,
    NOOS_EXC_SVC = 2,
    NOOS_EXC_PREFETCH_ABORT = 3,
    NOOS_EXC_DATA_ABORT = 4,
    NOOS_EXC_RESERVED = 5,
    NOOS_EXC_IRQ = 6,
    NOOS_EXC_FIQ = 7,
};

struct noos_exception_frame {
    uint32_t r[13];
    uint32_t lr;
};

struct noos_fault_state {
    enum noos_exception_type type;
    struct noos_exception_frame frame;
    uint32_t spsr;
    uint32_t fault_pc;
    uint32_t dfsr;
    uint32_t dfar;
    uint32_t ifsr;
    uint32_t ifar;
};

_Noreturn void exception_handle(uint32_t type, uint32_t spsr,
                                const struct noos_exception_frame *frame);
void exception_irq_handle(uint32_t spsr,
                          const struct noos_exception_frame *frame);
void arm_irq_enable(void);
void arm_irq_disable(void);
const struct noos_fault_state *exception_last_fault(void);

#endif
