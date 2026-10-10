#ifndef NOOS_MMIO_H
#define NOOS_MMIO_H

#include <stdint.h>
#include <stddef.h>

static inline uint32_t mmio_read32(uintptr_t address)
{
    return *(volatile uint32_t *)address;
}

static inline void mmio_write32(uintptr_t address, uint32_t value)
{
    *(volatile uint32_t *)address = value;
}

static inline void compiler_barrier(void)
{
    __asm__ volatile ("" ::: "memory");
}

#endif
