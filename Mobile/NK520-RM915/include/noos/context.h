#ifndef NOOS_CONTEXT_H
#define NOOS_CONTEXT_H

#include <stdint.h>

struct noos_context {
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t sp;
    uint32_t lr;
};

void arm_context_switch(struct noos_context *from,
                        const struct noos_context *to);

#endif
