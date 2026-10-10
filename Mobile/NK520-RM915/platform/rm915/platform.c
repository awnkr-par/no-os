#include <noos/platform.h>

void platform_early_init(void)
{
    /*
     * Intentionally empty. Do not touch unknown MMIO during bring-up.
     * Each peripheral is enabled only after its address and clock/reset
     * requirements are verified from trustworthy hardware documentation.
     */
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
