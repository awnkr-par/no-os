#ifndef NOOS_HEAP_H
#define NOOS_HEAP_H

#include <stddef.h>
#include <stdint.h>

struct heap_stats {
    uint32_t pages_owned;
    uint32_t bytes_requested;
    uint32_t bytes_available_current_page;
};

void heap_init(void);
void *kmalloc(size_t size);
void *kcalloc(size_t count, size_t size);
const struct heap_stats *heap_get_stats(void);

#endif
