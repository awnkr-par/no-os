#include <noos/boot.h>
#include <noos/console.h>
#include <noos/panic.h>
#include <noos/platform.h>
#include <noos/pmm.h>

static void log_boot_info(const struct noos_boot_info *info)
{
    console_write("noOS RM-915\n");
    console_write("platform: ");
    console_write(platform_name());
    console_write("\n");

    console_write("boot r0: ");
    console_write_hex32(info->boot_r0);
    console_write("\nmachine: ");
    console_write_hex32(info->machine_type);
    console_write("\nboot data: ");
    console_write_hex32(info->tags_address);
    console_write("\nATAGS: ");
    console_write(info->atags_valid ? "valid" : "not detected");
    console_write("\n");

    if (info->memory_size != 0) {
        console_write("RAM start: ");
        console_write_hex32(info->memory_start);
        console_write("\nRAM bytes: ");
        console_write_dec32(info->memory_size);
        console_write("\n");
    }

    if (info->command_line != 0) {
        console_write("cmdline: ");
        console_write(info->command_line);
        console_write("\n");
    }
}

void kernel_main(uint32_t boot_r0, uint32_t machine_type, uint32_t boot_data)
{
    struct noos_boot_info info;

    console_init();
    platform_early_init();
    boot_info_init(&info, boot_r0, machine_type, boot_data);
    log_boot_info(&info);

    if (!info.atags_valid || info.memory_size == 0) {
        panic("no usable RAM map from bootloader");
    }

    if (!pmm_init(info.memory_start, info.memory_size)) {
        panic("physical memory manager initialization failed");
    }

    const struct pmm_stats *stats = pmm_get_stats();
    console_write("PMM free pages: ");
    console_write_dec32(stats->free_pages);
    console_write("\n");

    /*
     * The early log now contains a complete boot report even without UART.
     * A verified ttyHSL0 or framebuffer backend can replay this buffer later.
     */
    platform_idle();
}
