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

#define RM915_TLMM_BASE               0x00800000u
#define RM915_CLK_CTL_BASE            0x00900000u
#define RM915_TLMM_CFG_ADDR(gpio)     (RM915_TLMM_BASE + 0x1000u + (gpio) * 0x10u)
#define RM915_GSBI_HCLK_CTL(id)       (RM915_CLK_CTL_BASE + 0x29C0u + 0x20u * ((id) - 1u))
#define RM915_GSBI_UART_MD(id)        (RM915_CLK_CTL_BASE + 0x29D0u + 0x20u * ((id) - 1u))
#define RM915_GSBI_UART_NS(id)        (RM915_CLK_CTL_BASE + 0x29D4u + 0x20u * ((id) - 1u))
#define RM915_GSBI_RESET(id)          (RM915_CLK_CTL_BASE + 0x29DCu + 0x20u * ((id) - 1u))

#define RM915_UART_M                  3u
#define RM915_UART_N                  625u
#define RM915_UART_D                  1u
#define RM915_UART_PLL8_MUX           3u
#define RM915_UART_MD16(m, n)         (((m) << 16) | ((~(n)) & 0xFFFFu))
#define RM915_UART_NS_VALUE(n, m, d, s) \
    ((((~((n) - (m))) & 0xFFFFu) << 16) | \
     (2u << 5) | (((d) - 1u) << 3) | (s))

#define RM915_UART_MD_VALUE \
    RM915_UART_MD16(RM915_UART_M, RM915_UART_N)
#define RM915_UART_NS_VALUE_1843200 \
    RM915_UART_NS_VALUE(RM915_UART_N, RM915_UART_M, RM915_UART_D, RM915_UART_PLL8_MUX)

_Static_assert(RM915_TLMM_CFG_ADDR(22u) == 0x00801160u,
               "RM915 TX TLMM address mismatch");
_Static_assert(RM915_TLMM_CFG_ADDR(23u) == 0x00801170u,
               "RM915 RX TLMM address mismatch");
_Static_assert(RM915_GSBI_HCLK_CTL(5u) == 0x00902A40u,
               "RM915 GSBI5 HCLK address mismatch");
_Static_assert(RM915_GSBI_UART_MD(5u) == 0x00902A50u,
               "RM915 GSBI5 MD address mismatch");
_Static_assert(RM915_GSBI_UART_NS(5u) == 0x00902A54u,
               "RM915 GSBI5 NS address mismatch");
_Static_assert(RM915_GSBI_RESET(5u) == 0x00902A5Cu,
               "RM915 GSBI5 reset address mismatch");
_Static_assert(RM915_UART_MD_VALUE == 0x0003FD8Eu,
               "RM915 1.8432 MHz UART MD value mismatch");
_Static_assert(RM915_UART_NS_VALUE_1843200 == 0xFD910043u,
               "RM915 1.8432 MHz UART NS value mismatch");

static struct uartdm_stats g_uart;

/*
 * Verified from Android4Lumia bootloader_msm8227, L520_stable:
 *
 *   target/msm8960/init.c
 *   platform/msm8960/gpio.c
 *   platform/msm8960/include/platform/{iomap,clock}.h
 *   platform/msm8960/clock.c
 *   platform/msm_shared/{uart_dm.c,include/gsbi.h}
 *
 * This is a DATA PLAN ONLY. noOS does not execute these clock/TLMM writes.
 */
static const struct uartdm_full_init_plan g_full_plan = {
    .tlmm_tx_cfg = RM915_TLMM_CFG_ADDR(NOOS_RM915_UART_TX_GPIO),
    .tlmm_rx_cfg = RM915_TLMM_CFG_ADDR(NOOS_RM915_UART_RX_GPIO),
    .tlmm_cfg_value = 0x000000C4u,

    .gsbi_ctrl = 0x16400000u,
    .gsbi_protocol_value = 0x00000060u,

    .clk_hclk_ctl = RM915_GSBI_HCLK_CTL(NOOS_RM915_GSBI_ID),
    .clk_uart_md = RM915_GSBI_UART_MD(NOOS_RM915_GSBI_ID),
    .clk_uart_ns = RM915_GSBI_UART_NS(NOOS_RM915_GSBI_ID),
    .clk_reset = RM915_GSBI_RESET(NOOS_RM915_GSBI_ID),

    /*
     * 1.8432 MHz GSBI UART clock:
     *   F_GSBI_UART(1843200, pll8, 1, 3, 625)
     *   MD16(3, 625) = 0x0003FD8E
     *   NS(...625,3,...d=1,...pll8 mux=3) = 0xFD910043
     */
    .uart_md_value = RM915_UART_MD_VALUE,
    .uart_ns_value = RM915_UART_NS_VALUE_1843200,
    .uart_target_hz = 1843200u,
    .uart_baud = 115200u,
};

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

const struct uartdm_full_init_plan *uartdm_full_init_plan(void)
{
    return &g_full_plan;
}

bool uartdm_validate_full_init_plan(void)
{
    const uint32_t expected_tx_cfg =
        0x00800000u + 0x1000u + NOOS_RM915_UART_TX_GPIO * 0x10u;
    const uint32_t expected_rx_cfg =
        0x00800000u + 0x1000u + NOOS_RM915_UART_RX_GPIO * 0x10u;

    const uint32_t expected_hclk =
        0x00900000u + 0x29C0u + 0x20u * (NOOS_RM915_GSBI_ID - 1u);
    const uint32_t expected_md =
        0x00900000u + 0x29D0u + 0x20u * (NOOS_RM915_GSBI_ID - 1u);
    const uint32_t expected_ns =
        0x00900000u + 0x29D4u + 0x20u * (NOOS_RM915_GSBI_ID - 1u);
    const uint32_t expected_reset =
        0x00900000u + 0x29DCu + 0x20u * (NOOS_RM915_GSBI_ID - 1u);

    const uint32_t md16 =
        (3u << 16) | ((~625u) & 0xFFFFu);
    const uint32_t ns =
        (((~(625u - 3u)) & 0xFFFFu) << 16) |
        (2u << 5) |
        3u;

    return
        g_full_plan.tlmm_tx_cfg == expected_tx_cfg &&
        g_full_plan.tlmm_rx_cfg == expected_rx_cfg &&
        g_full_plan.tlmm_cfg_value == 0xC4u &&
        g_full_plan.gsbi_ctrl == NOOS_RM915_GSBI_BASE &&
        g_full_plan.gsbi_protocol_value == 0x60u &&
        g_full_plan.clk_hclk_ctl == expected_hclk &&
        g_full_plan.clk_uart_md == expected_md &&
        g_full_plan.clk_uart_ns == expected_ns &&
        g_full_plan.clk_reset == expected_reset &&
        g_full_plan.uart_md_value == md16 &&
        g_full_plan.uart_ns_value == ns &&
        g_full_plan.uart_target_hz == 1843200u &&
        g_full_plan.uart_baud == 115200u;
}
