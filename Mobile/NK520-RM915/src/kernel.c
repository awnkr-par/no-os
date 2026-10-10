#include <noos/boot.h>
#include <noos/platform.h>

void kernel_main(uint32_t boot_r0, uint32_t machine_type, uint32_t boot_data)
{
    struct noos_boot_info info;

    boot_info_init(&info, boot_r0, machine_type, boot_data);
    platform_early_init();

    /*
     * Milestone 1: noOS now preserves and parses the ARM32 boot handoff.
     * The parsed state remains available through boot_info_get() for the
     * first debug transport, memory allocator, and platform probes.
     */
    platform_idle();
}
