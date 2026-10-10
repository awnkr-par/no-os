#include <noos/bootargs.h>
#include <noos/console.h>
#include <noos/heap.h>
#include <noos/klog.h>
#include <noos/memory.h>
#include <noos/mmu.h>
#include <noos/pmm.h>
#include <noos/scheduler.h>
#include <noos/selftest.h>
#include <noos/uartdm.h>
#include <noos/timer.h>

static void record(struct selftest_result *result, bool ok, const char *name)
{
    console_printf("[selftest] %s: %s\n", name, ok ? "ok" : "FAIL");

    if (ok) {
        result->passed++;
    } else {
        result->failed++;
    }
}

static bool reserved_ranges_protected(const struct noos_boot_info *boot)
{
    const struct pmm_stats *stats = pmm_get_stats();

    for (uint32_t i = 0; i < boot->reserved_range_count; ++i) {
        const struct noos_reserved_range *range = &boot->reserved_ranges[i];

        if (range->size == 0) {
            continue;
        }

        const uint32_t end = range->start + range->size;
        if (end <= stats->region_start || range->start >= stats->region_end) {
            continue;
        }

        uint32_t probe = range->start;
        if (probe < stats->region_start) {
            probe = stats->region_start;
        }

        if (!pmm_page_is_permanent(probe)) {
            return false;
        }
    }

    return true;
}

static void dummy_task(void *context)
{
    (void)context;
}

static bool scheduler_core_test(void)
{
    scheduler_init();

    const int32_t a = scheduler_add(dummy_task, 0);
    const int32_t b = scheduler_add(dummy_task, 0);
    if (a <= 0 || b <= 0 || a == b) {
        return false;
    }

    const int32_t first = scheduler_pick_next();
    const int32_t second = scheduler_pick_next();
    if (first <= 0 || second <= 0 || first == second) {
        return false;
    }

    if (!scheduler_sleep_until((uint32_t)first, 2u)) {
        return false;
    }

    scheduler_tick();
    const struct noos_task *sleeping = scheduler_task((uint32_t)first);
    if (sleeping == 0 || sleeping->state != NOOS_TASK_SLEEPING) {
        return false;
    }

    scheduler_tick();
    sleeping = scheduler_task((uint32_t)first);
    return sleeping != 0 && sleeping->state == NOOS_TASK_READY;
}

struct selftest_result selftest_run(const struct noos_boot_info *boot,
                                    const struct noos_boot_options *options)
{
    struct selftest_result result = {0, 0};

    uint8_t a[16];
    uint8_t b[16];

    memset(a, 0xA5, sizeof(a));
    memset(b, 0x00, sizeof(b));
    memcpy(b, a, sizeof(a));
    record(&result, memcmp(a, b, sizeof(a)) == 0, "memcpy/memcmp");

    for (uint32_t i = 0; i < sizeof(a); ++i) {
        a[i] = (uint8_t)i;
    }
    memmove(&a[2], &a[0], 10);
    record(&result, a[2] == 0 && a[11] == 9, "memmove overlap");

    const struct pmm_stats before = *pmm_get_stats();
    void *page = pmm_alloc_page();
    const struct pmm_stats allocated = *pmm_get_stats();
    bool page_ok = page != 0 &&
                   ((uintptr_t)page % NOOS_PAGE_SIZE) == 0 &&
                   allocated.free_pages + 1u == before.free_pages;
    record(&result, page_ok, "PMM allocation");

    bool free_ok = page_ok && pmm_free_page(page);
    const struct pmm_stats freed = *pmm_get_stats();
    free_ok = free_ok && freed.free_pages == before.free_pages;
    record(&result, free_ok, "PMM free/reuse accounting");

    record(&result, page_ok && !pmm_free_page(page), "PMM double-free rejection");
    record(&result, boot != 0 && reserved_ranges_protected(boot),
           "boot reserved ranges protected");

    uint32_t *words = (uint32_t *)kcalloc(16, sizeof(uint32_t));
    bool zeroed = words != 0;
    if (zeroed) {
        for (uint32_t i = 0; i < 16; ++i) {
            if (words[i] != 0) {
                zeroed = false;
                break;
            }
        }
    }
    record(&result, zeroed, "heap calloc");

    void *unaligned_size = kmalloc(13);
    record(&result, unaligned_size != 0 &&
           (((uintptr_t)unaligned_size & 7u) == 0), "heap alignment");

    struct noos_boot_options parsed_options;
    bootargs_parse(&parsed_options,
                   "noos.splash=0 noos.selftest=yes noos.mmu-plan=off "
                   "noos.sched-test=1 noos.loglevel=0x4");
    record(&result,
           !parsed_options.splash && parsed_options.selftest &&
           !parsed_options.mmu_plan && parsed_options.scheduler_test &&
           parsed_options.log_level == 4u,
           "boot option parsing");

    if (options == 0 || options->scheduler_test) {
        record(&result, scheduler_core_test(), "scheduler core state machine");
    }

    char log_tail[64];
    const size_t snap = klog_snapshot(log_tail, sizeof(log_tail));
    record(&result, snap != 0 && klog_get_stats()->bytes_written != 0,
           "circular kernel log");

    record(&result, uartdm_validate_full_init_plan(),
           "RM915 UART full-init constant plan");

    if (options == 0 || options->mmu_plan) {
        record(&result,
               mmu_descriptor_is_device(mmu_l1_descriptor(0x00800000u)) &&
               mmu_descriptor_is_device(mmu_l1_descriptor(0x00900000u)) &&
               mmu_descriptor_is_device(mmu_l1_descriptor(0x16400000u)),
               "RM915 UART MMIO device mappings");
    }

    record(&result, rm915_timer_validate_plan(),
           "RM915 timer/GIC constant plan");

    if (options == 0 || options->mmu_plan) {
        record(&result,
               mmu_descriptor_is_device(mmu_l1_descriptor(0x02000000u)),
               "RM915 timer/GIC MMIO device mapping");
    }

    return result;
}

bool selftest_ok(struct selftest_result result)
{
    return result.failed == 0;
}
