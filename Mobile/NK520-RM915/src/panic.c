#include <noos/console.h>
#include <noos/framebuffer.h>
#include <noos/panic.h>
#include <noos/status.h>

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
    boot_status_set(NOOS_STAGE_PANIC, 0);

    console_write("PANIC: ");
    console_write(reason != 0 ? reason : "(no reason)");
    console_write("\n");

    if (framebuffer_available()) {
        framebuffer_show_boot_splash(false);
    }

    halt_forever();
}
