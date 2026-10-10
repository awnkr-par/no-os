#include <noos/memory.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    for (size_t i = 0; i < count; ++i) {
        d[i] = s[i];
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d == s || count == 0) {
        return dest;
    }

    if (d < s) {
        for (size_t i = 0; i < count; ++i) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = count; i != 0; --i) {
            d[i - 1] = s[i - 1];
        }
    }

    return dest;
}

void *memset(void *dest, int value, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t byte = (uint8_t)value;

    for (size_t i = 0; i < count; ++i) {
        d[i] = byte;
    }

    return dest;
}

int memcmp(const void *lhs, const void *rhs, size_t count)
{
    const uint8_t *a = (const uint8_t *)lhs;
    const uint8_t *b = (const uint8_t *)rhs;

    for (size_t i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            return (int)a[i] - (int)b[i];
        }
    }

    return 0;
}
