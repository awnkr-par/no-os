#include <noos/console.h>
#include <noos/heap.h>
#include <noos/memory.h>
#include <noos/pmm.h>
#include <noos/selftest.h>

static void record(struct selftest_result *result, bool ok, const char *name)
{
    console_write("[selftest] ");
    console_write(name);
    console_write(ok ? ": ok\n" : ": FAIL\n");

    if (ok) {
        result->passed++;
    } else {
        result->failed++;
    }
}

struct selftest_result selftest_run(void)
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

    return result;
}

bool selftest_ok(struct selftest_result result)
{
    return result.failed == 0;
}
