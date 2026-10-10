#include <noos/boot.h>
#include <noos/bootargs.h>
#include <noos/console.h>
#include <noos/cpu.h>
#include <noos/diagnostic.h>
#include <noos/framebuffer.h>
#include <noos/heap.h>
#include <noos/irq.h>
#include <noos/klog.h>
#include <noos/mmu.h>
#include <noos/panic.h>
#include <noos/platform.h>
#include <noos/pmm.h>
#include <noos/scheduler.h>
#include <noos/selftest.h>
#include <noos/status.h>
#include <noos/uartdm.h>

static const char *boot_protocol_name(enum noos_boot_protocol protocol)
{
    switch (protocol) {
    case NOOS_BOOT_ATAGS: return "ATAGS";
    case NOOS_BOOT_DTB: return "DTB";
    default: return "unknown";
    }
}

static const char *pixel_format_name(enum noos_pixel_format format)
{
    switch (format) {
    case NOOS_PIXEL_XRGB8888: return "xrgb8888";
    case NOOS_PIXEL_ARGB8888: return "argb8888";
    case NOOS_PIXEL_RGB565: return "rgb565";
    default: return "unknown";
    }
}

static const char *reserved_kind_name(enum noos_reserved_kind kind)
{
    switch (kind) {
    case NOOS_RESERVED_BOOT_DATA: return "boot-data";
    case NOOS_RESERVED_FRAMEBUFFER: return "framebuffer";
    case NOOS_RESERVED_INITRD: return "initrd";
    case NOOS_RESERVED_DTB_MEMRESERVE: return "dtb-memreserve";
    case NOOS_RESERVED_DTB_NODE: return "dtb-reserved";
    default: return "generic";
    }
}

static void log_cpu_state(void)
{
    struct cpu_state state;
    cpu_read_state(&state);
    diagnostic_capture_cpu(&state);

    console_write("CPU MIDR: ");
    console_write_hex32(state.midr);
    console_write("\nCPSR: ");
    console_write_hex32(state.cpsr);
    console_write("\nSCTLR: ");
    console_write_hex32(state.sctlr);
    console_write(" MMU=");
    console_write(cpu_mmu_enabled(&state) ? "on" : "off");
    console_write(" D-cache=");
    console_write(cpu_dcache_enabled(&state) ? "on" : "off");
    console_write(" I-cache=");
    console_write(cpu_icache_enabled(&state) ? "on" : "off");
    console_write("\n");
}

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

    console_write("reserved ranges: ");
    console_write_dec32(info->reserved_range_count);
    console_write("\n");

    for (uint32_t i = 0; i < info->reserved_range_count; ++i) {
        const struct noos_reserved_range *range = &info->reserved_ranges[i];
        console_write("  reserve ");
        console_write(reserved_kind_name(range->kind));
        console_write(": ");
        console_write_hex32(range->start);
        console_write(" + ");
        console_write_hex32(range->size);
        console_write("\n");
    }

    if (info->memory_truncated || info->reserved_truncated) {
        console_write("WARNING: boot memory metadata truncated\n");
    }

    if (info->initrd_size != 0) {
        console_write("initrd: ");
        console_write_hex32(info->initrd_start);
        console_write(" + ");
        console_write_hex32(info->initrd_size);
        console_write("\n");
    }
}

static void log_framebuffer(const struct noos_boot_info *info)
{
    if (!info->framebuffer.valid) {
        console_write("framebuffer: not supplied\n");
        return;
    }

    console_write("framebuffer: ");
    console_write_hex32(info->framebuffer.address);
    console_write(" ");
    console_write_dec32(info->framebuffer.width);
    console_write("x");
    console_write_dec32(info->framebuffer.height);
    console_write(" stride=");
    console_write_dec32(info->framebuffer.stride);
    console_write(" format=");
    console_write(pixel_format_name(info->framebuffer.format));
    console_write("\n");
}

static void log_boot_info(const struct noos_boot_info *info)
{
    console_write("\nnoOS RM-915\n");
    console_write("platform: ");
    console_write(platform_name());
    console_write("\n");
    log_cpu_state();

    console_write("boot protocol: ");
    console_write(boot_protocol_name(info->protocol));
    console_write("\nboot r0: ");
    console_write_hex32(info->boot_r0);
    console_write("\nmachine: ");
    console_write_hex32(info->machine_type);
    console_write("\nboot data: ");
    console_write_hex32(info->boot_data_address);
    console_write(" + ");
    console_write_hex32(info->boot_data_size);
    console_write("\nboot data valid: ");
    console_write(info->boot_data_valid ? "yes" : "no");
    console_write("\n");

    log_memory_map(info);
    log_framebuffer(info);

    if (info->command_line != 0) {
        console_write("cmdline: ");
        console_write(info->command_line);
        console_write("\n");
    }
}

static void log_mmu_state(void)
{
    const struct mmu_stats *mmu = mmu_get_stats();

    console_write("MMU plan table: ");
    console_write_hex32(mmu->table_address);
    console_write("\nmapped 1MiB sections: ");
    console_write_dec32(mmu->mapped_sections);
    console_write(" RAM=");
    console_write_dec32(mmu->ram_sections);
    console_write(" DEV=");
    console_write_dec32(mmu->device_sections);
    console_write(" range=");
    console_write_hex32(mmu->first_mapped_section << 20);
    console_write("-");
    console_write_hex32((mmu->last_mapped_section << 20) | 0xFFFFFu);
    console_write("\nMMU plan validated: ");
    console_write(mmu->validated ? "yes" : "no");
    console_write(" (not enabled)\n");
}

static void log_allocator_state(void)
{
    const struct pmm_stats *pmm = pmm_get_stats();
    const struct heap_stats *heap = heap_get_stats();

    console_write("PMM span: ");
    console_write_hex32(pmm->region_start);
    console_write(" - ");
    console_write_hex32(pmm->region_end);
    console_write("\nPMM pages total/usable/free/permanent/allocated: ");
    console_write_dec32(pmm->total_pages);
    console_write("/");
    console_write_dec32(pmm->usable_pages);
    console_write("/");
    console_write_dec32(pmm->free_pages);
    console_write("/");
    console_write_dec32(pmm->permanent_pages);
    console_write("/");
    console_write_dec32(pmm->allocated_pages);
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
    diagnostic_init();
    boot_status_init();
    platform_early_init();

    if (platform_debug_init()) {
        console_set_sink(platform_debug_putc);
        console_write("debug transport: LK GSBI5 UARTDM handoff\n");
    }

    boot_info_init(&info, boot_r0, machine_type, boot_data);
    diagnostic_capture_boot(&info);
    boot_status_set(NOOS_STAGE_BOOT_INFO, (uint32_t)info.protocol);

    struct noos_boot_options options;
    bootargs_parse(&options, info.command_line);
    diagnostic_capture_boot_options(&options);
    console_printf("boot options: splash=%s selftest=%s mmu-plan=%s sched-test=%s loglevel=%u\n",
                   options.splash ? "on" : "off",
                   options.selftest ? "on" : "off",
                   options.mmu_plan ? "on" : "off",
                   options.scheduler_test ? "on" : "off",
                   options.log_level);

    const bool have_framebuffer = framebuffer_init(&info.framebuffer);
    if (have_framebuffer && options.splash) {
        framebuffer_show_booting();
    }

    log_boot_info(&info);

    if (!info.boot_data_valid || info.memory_bank_count == 0) {
        panic("no usable RAM map from bootloader");
    }

    if (!pmm_init(&info)) {
        panic("physical memory manager initialization failed");
    }
    diagnostic_capture_pmm(pmm_get_stats());
    boot_status_set(NOOS_STAGE_MEMORY, pmm_get_stats()->free_pages);

    heap_init();
    boot_status_set(NOOS_STAGE_HEAP, 0);
    log_allocator_state();

    if (options.mmu_plan) {
        if (!mmu_prepare_identity(&info)) {
            panic("MMU identity-map planning failed");
        }
        diagnostic_capture_mmu(mmu_get_stats());
        boot_status_set(NOOS_STAGE_MMU_PLAN, mmu_get_stats()->mapped_sections);
        log_mmu_state();
    } else {
        console_write("MMU plan: disabled by boot option\n");
    }

    irq_init();
    scheduler_init();

    struct selftest_result tests = {0, 0};
    if (options.selftest) {
        tests = selftest_run(&info, &options);
        diagnostic_capture_selftest(tests);
        boot_status_set(NOOS_STAGE_SELFTEST, tests.failed);
        console_printf("selftests passed/failed: %u/%u\n", tests.passed, tests.failed);

        if (!selftest_ok(tests)) {
            panic("kernel self-test failed");
        }
    } else {
        console_write("selftests: disabled by boot option\n");
    }

    if (have_framebuffer && options.splash) {
        framebuffer_show_runtime(&info, pmm_get_stats(), tests);
    }

    log_allocator_state();
    diagnostic_capture_pmm(pmm_get_stats());
    diagnostic_capture_uart(uartdm_get_stats());
    diagnostic_capture_scheduler(scheduler_get_stats());
    diagnostic_capture_irq(irq_get_stats());
    const struct klog_stats *log_stats = klog_get_stats();
    console_printf("klog: bytes=%u wraps=%u overwritten=%u retained=%u\n",
                   log_stats->bytes_written, log_stats->wraps,
                   log_stats->bytes_dropped, log_stats->length);
    console_write("kernel bring-up milestone complete; entering idle\n");
    boot_status_set(NOOS_STAGE_READY, log_stats->length);
    diagnostic_capture_console();

    platform_idle();
}
