#include <noos/pmm.h>
#include <stddef.h>
#include <stdint.h>

extern uint8_t __image_end[];

static struct pmm_stats g_stats;
static bool g_ready;

static uint32_t align_up_page(uint32_t value)
{
    return (value + (NOOS_PAGE_SIZE - 1u)) & ~(NOOS_PAGE_SIZE - 1u);
}

bool pmm_init(uint32_t memory_start, uint32_t memory_size)
{
    const uint32_t memory_end = memory_start + memory_size;
    const uint32_t image_end = (uint32_t)(uintptr_t)__image_end;
    const uint32_t first_free = align_up_page(image_end);

    g_ready = false;
    g_stats.region_start = memory_start;
    g_stats.region_end = memory_end;
    g_stats.next_free = first_free;
    g_stats.total_pages = 0;
    g_stats.free_pages = 0;

    if (memory_size < NOOS_PAGE_SIZE || memory_end <= memory_start) {
        return false;
    }

    if (first_free < memory_start || first_free >= memory_end) {
        return false;
    }

    g_stats.total_pages = (memory_end - memory_start) / NOOS_PAGE_SIZE;
    g_stats.free_pages = (memory_end - first_free) / NOOS_PAGE_SIZE;
    g_ready = true;
    return true;
}

void *pmm_alloc_page(void)
{
    if (!g_ready || g_stats.free_pages == 0) {
        return 0;
    }

    const uint32_t page = g_stats.next_free;
    g_stats.next_free += NOOS_PAGE_SIZE;
    g_stats.free_pages--;
    return (void *)(uintptr_t)page;
}

const struct pmm_stats *pmm_get_stats(void)
{
    return &g_stats;
}
