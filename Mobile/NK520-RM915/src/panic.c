#include <noos/panic.h>

_Noreturn void halt_forever(void)
{
    for (;;) {
        __asm__ volatile ("wfi");
    }
}

_Noreturn void panic(const char *reason)
{
    (void)reason;

    /* A diagnostic backend will consume this once UART/display is verified. */
    halt_forever();
}
