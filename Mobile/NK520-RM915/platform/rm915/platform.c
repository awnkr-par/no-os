#include <noos/platform.h>
#include <noos/uartdm.h>

#ifndef NOOS_UARTDM_HANDOFF
#define NOOS_UARTDM_HANDOFF 0
#endif

void platform_early_init(void)
{
    /*
     * Keep unconditional platform bring-up side-effect free.
     * Hardware transports are explicitly opt-in while first-boot behavior
     * is still being validated on a real RM-915.
     */
}

bool platform_debug_init(void)
{
#if NOOS_UARTDM_HANDOFF
    return uartdm_adopt_bootloader();
#else
    return false;
#endif
}

void platform_debug_putc(char c)
{
#if NOOS_UARTDM_HANDOFF
    (void)uartdm_putc(c);
#else
    (void)c;
#endif
}

const char *platform_name(void)
{
    return "Nokia Lumia 520 (RM-915)";
}

_Noreturn void platform_idle(void)
{
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
