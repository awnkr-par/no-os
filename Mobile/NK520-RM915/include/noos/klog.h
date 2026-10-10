#ifndef NOOS_KLOG_H
#define NOOS_KLOG_H

#include <stddef.h>
#include <stdint.h>

#define NOOS_KLOG_CAPACITY 8192u

struct klog_stats {
    uint32_t bytes_written;
    uint32_t bytes_dropped;
    uint32_t wraps;
    uint32_t length;
};

void klog_init(void);
void klog_putc(char c);
size_t klog_snapshot(char *dest, size_t capacity);
const struct klog_stats *klog_get_stats(void);

#endif
