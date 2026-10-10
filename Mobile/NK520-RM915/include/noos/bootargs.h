#ifndef NOOS_BOOTARGS_H
#define NOOS_BOOTARGS_H

#include <stdbool.h>
#include <stdint.h>

struct noos_boot_options {
    bool splash;
    bool selftest;
    bool mmu_plan;
    bool scheduler_test;
    bool timer_irq;
    uint32_t log_level;
};

void bootargs_defaults(struct noos_boot_options *options);
void bootargs_parse(struct noos_boot_options *options, const char *command_line);
bool bootargs_get_bool(const char *command_line, const char *key, bool fallback);
uint32_t bootargs_get_u32(const char *command_line, const char *key, uint32_t fallback);

#endif
