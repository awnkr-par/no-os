#include <noos/klog.h>

static char g_ring[NOOS_KLOG_CAPACITY];
static uint32_t g_head;
static uint32_t g_length;
static struct klog_stats g_stats;

void klog_init(void)
{
    g_head = 0;
    g_length = 0;
    g_stats.bytes_written = 0;
    g_stats.bytes_dropped = 0;
    g_stats.wraps = 0;
    g_stats.length = 0;
}

void klog_putc(char c)
{
    g_ring[g_head] = c;
    g_head++;

    if (g_head == NOOS_KLOG_CAPACITY) {
        g_head = 0;
        g_stats.wraps++;
    }

    if (g_length < NOOS_KLOG_CAPACITY) {
        g_length++;
    } else {
        g_stats.bytes_dropped++;
    }

    g_stats.bytes_written++;
    g_stats.length = g_length;
}

size_t klog_snapshot(char *dest, size_t capacity)
{
    if (dest == 0 || capacity == 0) {
        return 0;
    }

    size_t copy = g_length;
    if (copy + 1u > capacity) {
        copy = capacity - 1u;
    }

    uint32_t start = (g_head + NOOS_KLOG_CAPACITY - g_length) % NOOS_KLOG_CAPACITY;
    if (copy < g_length) {
        start = (g_head + NOOS_KLOG_CAPACITY - (uint32_t)copy) % NOOS_KLOG_CAPACITY;
    }

    for (size_t i = 0; i < copy; ++i) {
        dest[i] = g_ring[(start + (uint32_t)i) % NOOS_KLOG_CAPACITY];
    }

    dest[copy] = '\0';
    return copy;
}

const struct klog_stats *klog_get_stats(void)
{
    return &g_stats;
}
