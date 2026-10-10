#include <noos/boot.h>
#include <noos/console.h>
#include <noos/heap.h>
#include <noos/panic.h>
#include <noos/platform.h>
#include <noos/pmm.h>
#include <noos/selftest.h>

static void log_memory_map(const struct noos_boot_info *info)
{
    console_write("RAM banks: ");
    console_write_dec32(info->memory_bank_count);
    console_write(" total bytes: ");
    console_write_dec32(info->total_memory_bytes);
    console_write("\n");

    for (uint32_t i = 0; i < info->memory_bank_count; ++i) {
        console_write("  bank ");
        console_write_dec32(i);
        console_write(": ");
        console_write_hex32(info->memory_banks[i].start);
        console_write(" + ");
        console_write_hex32(info->memory_banks[i].size);
        console_write("\n");
    }

    if (info->memory_truncated) {
        console_write("WARNING: ATAG memory bank list truncated\n");
    }
}

static void log_boot_info(const struct noos_boot_info *info)
{
    console_write("\nnoOS RM-915\n");
    console_write("platform: ");
    console_write(platform_name());
    console_write("\nboot r0: ");
    console_write_hex32(info->boot_r0);
    console_write("\nmachine: ");
    console_write_hex32(info->machine_type);
    console_write("\nboot data: ");
    console_write_hex32(info->tags_address);
    console_write("\nATAGS: ");
    console_write(info->atags_valid ? "valid" : "not detected");
    console_write("\n");

    log_memory_map(info);

    if (info->command_line != 0) {
        console_write("cmdline: ");
        console_write(info->command_line);
        console_write("\n");
    }
}

static void log_allocator_state(void)
{
    const struct pmm_stats *pmm = pmm_get_stats();
    const struct heap_stats *heap = heap_get_stats();

    console_write("PMM span: ");
    console_write_hex32(pmm->region_start);
    console_write(" - ");
    console_write_hex32(pmm->region_end);
    console_write("\nPMM pages total/usable/free/reserved: ");
    console_write_dec32(pmm->total_pages);
    console_write("/");
    console_write_dec32(pmm->usable_pages);
    console_write("/");
    console_write_dec32(pmm->free_pages);
    console_write("/");
    console_write_dec32(pmm->reserved_pages);
    console_write("\nheap pages: ");
    console_write_dec32(heap->pages_owned);
    console_write(" requested bytes: ");
    console_write_dec32(heap->bytes_requested);
    console_write("\n");
}

void kernel_main(uint32_t boot_r0, uint32_t machine_type, uint32_t boot_data)
{
    struct noos_boot_info info;

    console_init();
    platform_early_init();
    boot_info_init(&info, boot_r0, machine_type, boot_data);
    log_boot_info(&info);

    if (!info.atags_valid || info.memory_bank_count == 0) {
        panic("no usable RAM map from bootloader");
    }

    if (!pmm_init(&info)) {
        panic("physical memory manager initialization failed");
    }

    heap_init();
    log_allocator_state();

    const struct selftest_result tests = selftest_run();
    console_write("selftests passed/failed: ");
    console_write_dec32(tests.passed);
    console_write("/");
    console_write_dec32(tests.failed);
    console_write("\n");

    if (!selftest_ok(tests)) {
        panic("kernel self-test failed");
    }

    log_allocator_state();
    console_write("kernel bring-up milestone complete; entering idle\n");

    platform_idle();
}
