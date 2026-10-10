#include <noos/console.h>
#include <noos/diagnostic.h>
#include <noos/klog.h>
#include <noos/memory.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((section(".noos_diag"), used, aligned(64)))
static volatile struct noos_diagnostic_record g_diag;

static uint32_t checksum_record(const struct noos_diagnostic_record *record)
{
    const uint8_t *bytes = (const uint8_t *)record;
    uint32_t hash = 2166136261u;

    for (size_t i = 0; i < sizeof(*record); ++i) {
        if (i >= offsetof(struct noos_diagnostic_record, checksum) &&
            i < offsetof(struct noos_diagnostic_record, checksum) + sizeof(record->checksum)) {
            continue;
        }

        hash ^= bytes[i];
        hash *= 16777619u;
    }

    return hash;
}

static void refresh_checksum(void)
{
    struct noos_diagnostic_record snapshot;
    memcpy(&snapshot, (const void *)&g_diag, sizeof(snapshot));
    snapshot.checksum = 0;
    g_diag.checksum = checksum_record(&snapshot);
    __asm__ volatile ("" ::: "memory");
}

static void copy_string(char *dest, size_t capacity, const char *source)
{
    if (capacity == 0) {
        return;
    }

    size_t i = 0;
    if (source != 0) {
        while (i + 1u < capacity && source[i] != '\0') {
            dest[i] = source[i];
            i++;
        }
    }

    dest[i] = '\0';
    while (++i < capacity) {
        dest[i] = '\0';
    }
}

void diagnostic_init(void)
{
    memset((void *)&g_diag, 0, sizeof(g_diag));
    g_diag.magic = NOOS_DIAG_MAGIC;
    g_diag.version = NOOS_DIAG_VERSION;
    g_diag.size = sizeof(g_diag);
    refresh_checksum();
}

void diagnostic_set_stage(uint32_t stage, uint32_t detail)
{
    g_diag.stage = stage;
    g_diag.detail = detail;
    refresh_checksum();
}

void diagnostic_capture_boot(const struct noos_boot_info *boot)
{
    if (boot == 0) {
        return;
    }

    g_diag.boot_protocol = (uint32_t)boot->protocol;
    g_diag.boot_data_address = boot->boot_data_address;
    g_diag.boot_data_size = boot->boot_data_size;
    g_diag.total_memory_bytes = boot->total_memory_bytes;
    g_diag.reserved_range_count = boot->reserved_range_count;

    if (boot->framebuffer.valid) {
        g_diag.framebuffer_address = boot->framebuffer.address;
        g_diag.framebuffer_width = boot->framebuffer.width;
        g_diag.framebuffer_height = boot->framebuffer.height;
        g_diag.framebuffer_stride = boot->framebuffer.stride;
        g_diag.framebuffer_format = (uint32_t)boot->framebuffer.format;
    }

    refresh_checksum();
}

void diagnostic_capture_boot_options(const struct noos_boot_options *options)
{
    if (options == 0) {
        return;
    }

    g_diag.boot_option_flags =
        (options->splash ? 1u : 0u) |
        (options->selftest ? 2u : 0u) |
        (options->mmu_plan ? 4u : 0u) |
        (options->scheduler_test ? 8u : 0u);
    g_diag.boot_log_level = options->log_level;
    refresh_checksum();
}

void diagnostic_capture_cpu(const struct cpu_state *cpu)
{
    if (cpu == 0) {
        return;
    }

    g_diag.cpu_midr = cpu->midr;
    g_diag.cpu_cpsr = cpu->cpsr;
    g_diag.cpu_sctlr = cpu->sctlr;
    refresh_checksum();
}

void diagnostic_capture_pmm(const struct pmm_stats *pmm)
{
    if (pmm == 0) {
        return;
    }

    g_diag.pmm_total_pages = pmm->total_pages;
    g_diag.pmm_free_pages = pmm->free_pages;
    g_diag.pmm_permanent_pages = pmm->permanent_pages;
    g_diag.pmm_allocated_pages = pmm->allocated_pages;
    refresh_checksum();
}

void diagnostic_capture_mmu(const struct mmu_stats *mmu)
{
    if (mmu == 0) {
        return;
    }

    g_diag.mmu_table_address = mmu->table_address;
    g_diag.mmu_mapped_sections = mmu->mapped_sections;
    g_diag.mmu_first_section = mmu->first_mapped_section;
    g_diag.mmu_last_section = mmu->last_mapped_section;
    g_diag.mmu_validated = mmu->validated ? 1u : 0u;
    refresh_checksum();
}

void diagnostic_capture_uart(const struct uartdm_stats *uart)
{
    if (uart == 0) {
        return;
    }

    g_diag.uart_base = uart->base;
    g_diag.uart_bytes_sent = uart->bytes_sent;
    g_diag.uart_tx_timeouts = uart->tx_timeouts;
    g_diag.uart_flags = (uart->adopted ? 1u : 0u) |
                        (uart->healthy ? 2u : 0u);
    refresh_checksum();
}

void diagnostic_capture_selftest(struct selftest_result result)
{
    g_diag.selftests_passed = result.passed;
    g_diag.selftests_failed = result.failed;
    refresh_checksum();
}

void diagnostic_capture_scheduler(const struct scheduler_stats *scheduler)
{
    if (scheduler == 0) {
        return;
    }

    g_diag.scheduler_ticks = scheduler->ticks;
    g_diag.scheduler_tasks = scheduler->task_count;
    g_diag.scheduler_ready = scheduler->ready_count;
    g_diag.scheduler_sleeping = scheduler->sleeping_count;
    g_diag.scheduler_switches_planned = scheduler->switches_planned;
    refresh_checksum();
}

void diagnostic_capture_panic(const char *reason)
{
    copy_string((char *)g_diag.panic_reason, sizeof(g_diag.panic_reason), reason);
    diagnostic_capture_console();
}

void diagnostic_capture_console(void)
{
    char snapshot[NOOS_DIAG_LOG_BYTES];
    klog_snapshot(snapshot, sizeof(snapshot));
    copy_string((char *)g_diag.log_tail, sizeof(g_diag.log_tail), snapshot);

    const struct klog_stats *stats = klog_get_stats();
    g_diag.klog_bytes_written = stats->bytes_written;
    g_diag.klog_bytes_overwritten = stats->bytes_dropped;
    g_diag.klog_wraps = stats->wraps;
    g_diag.klog_length = stats->length;
    refresh_checksum();
}

const struct noos_diagnostic_record *diagnostic_get(void)
{
    return (const struct noos_diagnostic_record *)&g_diag;
}
