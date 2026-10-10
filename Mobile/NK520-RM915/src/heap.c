#include <noos/heap.h>
#include <noos/memory.h>
#include <noos/pmm.h>

static uintptr_t g_cursor;
static uintptr_t g_limit;
static struct heap_stats g_stats;

static uintptr_t align_up(uintptr_t value, uintptr_t alignment)
{
    return (value + alignment - 1u) & ~(alignment - 1u);
}

void heap_init(void)
{
    g_cursor = 0;
    g_limit = 0;
    g_stats.pages_owned = 0;
    g_stats.bytes_requested = 0;
    g_stats.bytes_available_current_page = 0;
}

void *kmalloc(size_t size)
{
    if (size == 0 || size > NOOS_PAGE_SIZE) {
        return 0;
    }

    const uintptr_t aligned_size = align_up((uintptr_t)size, 8u);
    uintptr_t start = align_up(g_cursor, 8u);

    if (start == 0 || start + aligned_size > g_limit) {
        void *page = pmm_alloc_page();
        if (page == 0) {
            return 0;
        }

        start = (uintptr_t)page;
        g_cursor = start;
        g_limit = start + NOOS_PAGE_SIZE;
        g_stats.pages_owned++;
    }

    void *result = (void *)start;
    g_cursor = start + aligned_size;
    g_stats.bytes_requested += (uint32_t)size;
    g_stats.bytes_available_current_page = (uint32_t)(g_limit - g_cursor);
    return result;
}

void *kcalloc(size_t count, size_t size)
{
    if (count == 0 || size == 0) {
        return 0;
    }

    const uint64_t total64 = (uint64_t)count * (uint64_t)size;
    if (total64 > (uint64_t)SIZE_MAX) {
        return 0;
    }

    const size_t total = (size_t)total64;
    void *ptr = kmalloc(total);
    if (ptr != 0) {
        memset(ptr, 0, total);
    }
    return ptr;
}

const struct heap_stats *heap_get_stats(void)
{
    return &g_stats;
}
