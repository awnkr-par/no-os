#ifndef NOOS_PMM_H
#define NOOS_PMM_H

#include <noos/boot.h>
#include <stdbool.h>
#include <stdint.h>

#define NOOS_PAGE_SIZE 4096u
#define NOOS_PMM_MAX_PAGES 131072u

struct pmm_stats {
    uint32_t region_start;
    uint32_t region_end;
    uint32_t total_pages;
    uint32_t usable_pages;
    uint32_t free_pages;
    uint32_t permanent_pages;
    uint32_t allocated_pages;
    uint32_t allocation_hint;
};

bool pmm_init(const struct noos_boot_info *boot);
void *pmm_alloc_page(void);
bool pmm_free_page(void *page);
bool pmm_reserve_range(uint32_t start, uint32_t size);
bool pmm_page_is_free(uint32_t address);
const struct pmm_stats *pmm_get_stats(void);

#endif
