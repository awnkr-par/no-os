#include <noos/console.h>
#include <noos/panic.h>

static const char *g_panic_reason;

_Noreturn void halt_forever(void)
{
    for (;;) {
        __asm__ volatile ("wfi");
    }
}

const char *panic_reason(void)
{
    return g_panic_reason;
}

_Noreturn void panic(const char *reason)
{
    g_panic_reason = reason;

    console_write("PANIC: ");
    console_write(reason != 0 ? reason : "(no reason)");
    console_write("\n");

    halt_forever();
}
