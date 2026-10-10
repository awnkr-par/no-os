#ifndef NOOS_BOOT_H
#define NOOS_BOOT_H

#include <noos/types.h>

#define NOOS_ATAG_CORE    0x54410001u
#define NOOS_ATAG_MEM     0x54410002u
#define NOOS_ATAG_CMDLINE 0x54410009u
#define NOOS_ATAG_NONE    0x00000000u
#define NOOS_MAX_MEMORY_BANKS 8u

enum noos_boot_protocol {
    NOOS_BOOT_UNKNOWN = 0,
    NOOS_BOOT_ATAGS = 1,
    NOOS_BOOT_DTB = 2
};

enum noos_pixel_format {
    NOOS_PIXEL_UNKNOWN = 0,
    NOOS_PIXEL_XRGB8888 = 1,
    NOOS_PIXEL_ARGB8888 = 2,
    NOOS_PIXEL_RGB565 = 3
};

struct noos_memory_bank {
    uint32_t start;
    uint32_t size;
};

struct noos_framebuffer {
    uint32_t address;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    enum noos_pixel_format format;
    bool valid;
};

struct noos_boot_info {
    uint32_t boot_r0;
    uint32_t machine_type;
    uint32_t boot_data_address;
    enum noos_boot_protocol protocol;
    struct noos_memory_bank memory_banks[NOOS_MAX_MEMORY_BANKS];
    uint32_t memory_bank_count;
    uint32_t total_memory_bytes;
    const char *command_line;
    struct noos_framebuffer framebuffer;
    bool boot_data_valid;
    bool memory_truncated;
};

void boot_info_init(struct noos_boot_info *info, uint32_t r0, uint32_t r1, uint32_t r2);
const struct noos_boot_info *boot_info_get(void);

#endif
