#include <noos/console.h>

#define EARLY_LOG_CAPACITY 4096u

static char g_early_log[EARLY_LOG_CAPACITY];
static size_t g_early_log_length;

static uint32_t divmod10(uint32_t value, uint32_t *remainder)
{
    uint32_t quotient = 0;
    uint32_t rem = 0;

    for (int bit = 31; bit >= 0; --bit) {
        rem = (rem << 1) | ((value >> (uint32_t)bit) & 1u);
        if (rem >= 10u) {
            rem -= 10u;
            quotient |= 1u << (uint32_t)bit;
        }
    }

    *remainder = rem;
    return quotient;
}

void console_init(void)
{
    g_early_log_length = 0;
    g_early_log[0] = '\0';
}

void console_putc(char c)
{
    if (g_early_log_length + 1u >= EARLY_LOG_CAPACITY) {
        return;
    }

    g_early_log[g_early_log_length++] = c;
    g_early_log[g_early_log_length] = '\0';
}

void console_write(const char *text)
{
    if (text == 0) {
        return;
    }

    while (*text != '\0') {
        console_putc(*text++);
    }
}

void console_write_hex32(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";

    console_write("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        console_putc(digits[(value >> (uint32_t)shift) & 0x0Fu]);
    }
}

void console_write_dec32(uint32_t value)
{
    char buffer[10];
    size_t length = 0;

    if (value == 0) {
        console_putc('0');
        return;
    }

    while (value != 0 && length < sizeof(buffer)) {
        uint32_t remainder;
        value = divmod10(value, &remainder);
        buffer[length++] = (char)('0' + remainder);
    }

    while (length != 0) {
        console_putc(buffer[--length]);
    }
}

const char *console_buffer(void)
{
    return g_early_log;
}

size_t console_buffer_length(void)
{
    return g_early_log_length;
}
