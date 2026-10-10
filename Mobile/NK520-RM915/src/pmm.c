#include <noos/pmm.h>
#include <noos/memory.h>
#include <stddef.h>
#include <stdint.h>

extern uint8_t __image_start[];
extern uint8_t __image_end[];

#define PMM_BITMAP_BYTES (NOOS_PMM_MAX_PAGES / 8u)

static uint8_t g_used_bitmap[PMM_BITMAP_BYTES];
static uint8_t g_permanent_bitmap[PMM_BITMAP_BYTES];
static struct pmm_stats g_stats;
static bool g_ready;

static uint32_t align_down(uint32_t value)
{
    return value & ~(NOOS_PAGE_SIZE - 1u);
}

static uint32_t align_up(uint32_t value)
{
    return (value + NOOS_PAGE_SIZE - 1u) & ~(NOOS_PAGE_SIZE - 1u);
}

static bool page_index(uint32_t address, uint32_t *index)
{
    if (address < g_stats.region_start || address >= g_stats.region_end) {
        return false;
    }

    *index = (address - g_stats.region_start) / NOOS_PAGE_SIZE;
    return *index < g_stats.total_pages;
}

static bool bitmap_test(const uint8_t *bitmap, uint32_t index)
{
    return (bitmap[index >> 3] & (uint8_t)(1u << (index & 7u))) != 0;
}

static void bitmap_set(uint8_t *bitmap, uint32_t index)
{
    bitmap[index >> 3] |= (uint8_t)(1u << (index & 7u));
}

static void bitmap_clear(uint8_t *bitmap, uint32_t index)
{
    bitmap[index >> 3] &= (uint8_t)~(1u << (index & 7u));
}

static bool page_used(uint32_t index)
{
    return bitmap_test(g_used_bitmap, index);
}

static bool page_permanent(uint32_t index)
{
    return bitmap_test(g_permanent_bitmap, index);
}

static void expose_usable_page(uint32_t address)
{
    uint32_t index;
    if (!page_index(address, &index)) {
        return;
    }

    if (page_used(index)) {
        bitmap_clear(g_used_bitmap, index);
        g_stats.usable_pages++;
        g_stats.free_pages++;
    }
}

static void reserve_page_permanently(uint32_t address)
{
    uint32_t index;
    if (!page_index(address, &index)) {
        return;
    }

    if (!page_permanent(index)) {
        bitmap_set(g_permanent_bitmap, index);
        g_stats.permanent_pages++;
    }

    if (!page_used(index)) {
        bitmap_set(g_used_bitmap, index);
        if (g_stats.free_pages != 0) {
            g_stats.free_pages--;
        }
    }
}

bool pmm_reserve_range(uint32_t start, uint32_t size)
{
    if (!g_ready || size == 0) {
        return false;
    }

    const uint32_t end_raw = start + size;
    if (end_raw < start) {
        return false;
    }

    uint32_t first = align_down(start);
    uint32_t end = align_up(end_raw);

    if (end <= g_stats.region_start || first >= g_stats.region_end) {
        return false;
    }

    if (first < g_stats.region_start) {
        first = g_stats.region_start;
    }
    if (end > g_stats.region_end) {
        end = g_stats.region_end;
    }

    for (uint32_t addr = first; addr < end; addr += NOOS_PAGE_SIZE) {
        reserve_page_permanently(addr);
    }
    return true;
}

bool pmm_init(const struct noos_boot_info *boot)
{
    memset(&g_stats, 0, sizeof(g_stats));
    memset(g_used_bitmap, 0xFF, sizeof(g_used_bitmap));
    memset(g_permanent_bitmap, 0x00, sizeof(g_permanent_bitmap));
    g_ready = false;

    if (boot == 0 || boot->memory_bank_count == 0) {
        return false;
    }

    uint32_t lowest = UINT32_MAX;
    uint32_t highest = 0;

    for (uint32_t i = 0; i < boot->memory_bank_count; ++i) {
        const uint32_t start = boot->memory_banks[i].start;
        const uint32_t size = boot->memory_banks[i].size;
        const uint32_t end = start + size;

        if (size == 0 || end <= start) {
            continue;
        }

        if (start < lowest) {
            lowest = start;
        }
        if (end > highest) {
            highest = end;
        }
    }

    if (lowest == UINT32_MAX || highest <= lowest) {
        return false;
    }

    g_stats.region_start = align_down(lowest);
    g_stats.region_end = align_up(highest);
    g_stats.total_pages = (g_stats.region_end - g_stats.region_start) / NOOS_PAGE_SIZE;

    if (g_stats.total_pages == 0 || g_stats.total_pages > NOOS_PMM_MAX_PAGES) {
        return false;
    }

    g_ready = true;

    for (uint32_t i = 0; i < boot->memory_bank_count; ++i) {
        const uint32_t raw_start = boot->memory_banks[i].start;
        const uint32_t raw_end = raw_start + boot->memory_banks[i].size;
        const uint32_t start = align_up(raw_start);
        const uint32_t end = align_down(raw_end);

        for (uint32_t addr = start; addr < end; addr += NOOS_PAGE_SIZE) {
            expose_usable_page(addr);
        }
    }

    pmm_reserve_range((uint32_t)(uintptr_t)__image_start,
                      (uint32_t)((uintptr_t)__image_end - (uintptr_t)__image_start));

    for (uint32_t i = 0; i < boot->reserved_range_count; ++i) {
        pmm_reserve_range(boot->reserved_ranges[i].start,
                          boot->reserved_ranges[i].size);
    }

    g_stats.allocation_hint = 0;
    return g_stats.free_pages != 0;
}

void *pmm_alloc_page(void)
{
    if (!g_ready || g_stats.free_pages == 0) {
        return 0;
    }

    for (uint32_t pass = 0; pass < 2; ++pass) {
        const uint32_t begin = pass == 0 ? g_stats.allocation_hint : 0;
        const uint32_t end = pass == 0 ? g_stats.total_pages : g_stats.allocation_hint;

        for (uint32_t index = begin; index < end; ++index) {
            if (!page_used(index)) {
                bitmap_set(g_used_bitmap, index);
                g_stats.free_pages--;
                g_stats.allocated_pages++;
                g_stats.allocation_hint = index + 1u;
                return (void *)(uintptr_t)(g_stats.region_start + index * NOOS_PAGE_SIZE);
            }
        }
    }

    return 0;
}

bool pmm_free_page(void *page)
{
    const uint32_t address = (uint32_t)(uintptr_t)page;
    uint32_t index;

    if (!g_ready || (address & (NOOS_PAGE_SIZE - 1u)) != 0 ||
        !page_index(address, &index) || !page_used(index) || page_permanent(index)) {
        return false;
    }

    bitmap_clear(g_used_bitmap, index);
    g_stats.free_pages++;
    if (g_stats.allocated_pages != 0) {
        g_stats.allocated_pages--;
    }

    if (index < g_stats.allocation_hint) {
        g_stats.allocation_hint = index;
    }
    return true;
}

bool pmm_page_is_free(uint32_t address)
{
    uint32_t index;
    if (!g_ready || !page_index(align_down(address), &index)) {
        return false;
    }
    return !page_used(index);
}

bool pmm_page_is_permanent(uint32_t address)
{
    uint32_t index;
    if (!g_ready || !page_index(align_down(address), &index)) {
        return false;
    }
    return page_permanent(index);
}

const struct pmm_stats *pmm_get_stats(void)
{
    return &g_stats;
}
