#ifndef NOOS_UARTDM_H
#define NOOS_UARTDM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NOOS_RM915_GSBI_ID        5u
#define NOOS_RM915_GSBI_BASE      0x16400000u
#define NOOS_RM915_UARTDM_BASE    0x16440000u
#define NOOS_RM915_UART_RX_GPIO   23u
#define NOOS_RM915_UART_TX_GPIO   22u
#define NOOS_RM915_UART_GPIO_FUNC 1u

struct uartdm_stats {
    uint32_t base;
    uint32_t bytes_sent;
    uint32_t tx_timeouts;
    bool adopted;
    bool healthy;
};

bool uartdm_adopt_bootloader(void);
bool uartdm_putc(char c);
void uartdm_write(const char *data, size_t length);
const struct uartdm_stats *uartdm_get_stats(void);

#endif
