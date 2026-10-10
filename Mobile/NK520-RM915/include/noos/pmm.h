#ifndef NOOS_PMM_H
#define NOOS_PMM_H

#include <stdbool.h>
#include <stdint.h>

#define NOOS_PAGE_SIZE 4096u

struct pmm_stats {
    uint32_t region_start;
    uint32_t region_end;
    uint32_t next_free;
    uint32_t total_pages;
    uint32_t free_pages;
};

bool pmm_init(uint32_t memory_start, uint32_t memory_size);
void *pmm_alloc_page(void);
const struct pmm_stats *pmm_get_stats(void);

#endif
