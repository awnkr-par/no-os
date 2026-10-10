#include <noos/mmio.h>
#include <noos/uartdm.h>

#define UARTDM_SR_OFFSET              0x008u
#define UARTDM_CR_OFFSET              0x010u
#define UARTDM_ISR_OFFSET             0x014u
#define UARTDM_NO_CHARS_TX_OFFSET     0x040u
#define UARTDM_TF_OFFSET              0x070u

#define UARTDM_SR_TXRDY               (1u << 2)
#define UARTDM_SR_TXEMT               (1u << 3)
#define UARTDM_ISR_TX_READY           (1u << 7)
#define UARTDM_GCMD_RESET_TX_READY    (3u << 8)

#define UARTDM_POLL_BUDGET            1000000u

static struct uartdm_stats g_uart;

static uint32_t reg(uint32_t offset)
{
    return NOOS_RM915_UARTDM_BASE + offset;
}

static bool wait_mask(uint32_t address, uint32_t mask)
{
    for (uint32_t i = 0; i < UARTDM_POLL_BUDGET; ++i) {
        if ((mmio_read32(address) & mask) != 0) {
            return true;
        }
    }

    g_uart.tx_timeouts++;
    g_uart.healthy = false;
    return false;
}

bool uartdm_adopt_bootloader(void)
{
    g_uart.base = NOOS_RM915_UARTDM_BASE;
    g_uart.bytes_sent = 0;
    g_uart.tx_timeouts = 0;
    g_uart.adopted = true;
    g_uart.healthy = true;

    /*
     * Do not change clocks, TLMM, GSBI protocol, MR1/MR2, or reset state.
     * The L520_stable LK target initializes GSBI5/UARTDM at 0x16440000
     * before chainloading us. This mode deliberately adopts that handoff.
     */
    return true;
}

static bool putc_raw(uint8_t value)
{
    if (!g_uart.adopted || !g_uart.healthy) {
        return false;
    }

    const uint32_t sr = mmio_read32(reg(UARTDM_SR_OFFSET));

    if ((sr & UARTDM_SR_TXEMT) == 0 &&
        !wait_mask(reg(UARTDM_ISR_OFFSET), UARTDM_ISR_TX_READY)) {
        return false;
    }

    mmio_write32(reg(UARTDM_NO_CHARS_TX_OFFSET), 1u);
    mmio_write32(reg(UARTDM_CR_OFFSET), UARTDM_GCMD_RESET_TX_READY);

    if (!wait_mask(reg(UARTDM_SR_OFFSET), UARTDM_SR_TXRDY)) {
        return false;
    }

    mmio_write32(reg(UARTDM_TF_OFFSET), (uint32_t)value);
    g_uart.bytes_sent++;
    return true;
}

bool uartdm_putc(char c)
{
    if (c == '\n') {
        if (!putc_raw((uint8_t)'\r')) {
            return false;
        }
    }

    return putc_raw((uint8_t)c);
}

void uartdm_write(const char *data, size_t length)
{
    if (data == 0) {
        return;
    }

    for (size_t i = 0; i < length; ++i) {
        if (!uartdm_putc(data[i])) {
            break;
        }
    }
}

const struct uartdm_stats *uartdm_get_stats(void)
{
    return &g_uart;
}
