#include <noos/pmm.h>
#include <noos/memory.h>
#include <stddef.h>
#include <stdint.h>

extern uint8_t __image_start[];
extern uint8_t __image_end[];

static uint8_t g_bitmap[NOOS_PMM_MAX_PAGES / 8u];
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

static bool bit_is_used(uint32_t index)
{
    return (g_bitmap[index >> 3] & (uint8_t)(1u << (index & 7u))) != 0;
}

static void bit_set_used(uint32_t index)
{
    g_bitmap[index >> 3] |= (uint8_t)(1u << (index & 7u));
}

static void bit_set_free(uint32_t index)
{
    g_bitmap[index >> 3] &= (uint8_t)~(1u << (index & 7u));
}

static bool mark_page_free(uint32_t address)
{
    uint32_t index;
    if (!page_index(address, &index)) {
        return false;
    }

    if (bit_is_used(index)) {
        bit_set_free(index);
        g_stats.free_pages++;
        g_stats.usable_pages++;
    }
    return true;
}

static bool mark_page_used(uint32_t address)
{
    uint32_t index;
    if (!page_index(address, &index)) {
        return false;
    }

    if (!bit_is_used(index)) {
        bit_set_used(index);
        if (g_stats.free_pages != 0) {
            g_stats.free_pages--;
        }
        g_stats.reserved_pages++;
    }
    return true;
}

bool pmm_reserve_range(uint32_t start, uint32_t size)
{
    if (!g_ready || size == 0) {
        return false;
    }

    uint32_t first = align_down(start);
    uint32_t end_raw = start + size;
    if (end_raw < start) {
        return false;
    }
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
        mark_page_used(addr);
    }
    return true;
}

bool pmm_init(const struct noos_boot_info *boot)
{
    memset(&g_stats, 0, sizeof(g_stats));
    memset(g_bitmap, 0xFF, sizeof(g_bitmap));
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
        uint32_t start = align_up(boot->memory_banks[i].start);
        const uint32_t raw_end = boot->memory_banks[i].start + boot->memory_banks[i].size;
        const uint32_t end = align_down(raw_end);

        for (uint32_t addr = start; addr < end; addr += NOOS_PAGE_SIZE) {
            mark_page_free(addr);
        }
    }

    pmm_reserve_range((uint32_t)(uintptr_t)__image_start,
                      (uint32_t)((uintptr_t)__image_end - (uintptr_t)__image_start));

    if (boot->tags_address != 0) {
        pmm_reserve_range(boot->tags_address, NOOS_PAGE_SIZE);
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
        uint32_t begin = pass == 0 ? g_stats.allocation_hint : 0;
        uint32_t end = pass == 0 ? g_stats.total_pages : g_stats.allocation_hint;

        for (uint32_t index = begin; index < end; ++index) {
            if (!bit_is_used(index)) {
                bit_set_used(index);
                g_stats.free_pages--;
                g_stats.reserved_pages++;
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

    if (!g_ready || (address & (NOOS_PAGE_SIZE - 1u)) != 0 || !page_index(address, &index)) {
        return false;
    }

    if (!bit_is_used(index)) {
        return false;
    }

    bit_set_free(index);
    g_stats.free_pages++;
    if (g_stats.reserved_pages != 0) {
        g_stats.reserved_pages--;
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
    return !bit_is_used(index);
}

const struct pmm_stats *pmm_get_stats(void)
{
    return &g_stats;
}
