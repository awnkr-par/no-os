#ifndef NOOS_BOOT_H
#define NOOS_BOOT_H

#include <noos/types.h>

#define NOOS_ATAG_CORE    0x54410001u
#define NOOS_ATAG_MEM     0x54410002u
#define NOOS_ATAG_CMDLINE 0x54410009u
#define NOOS_ATAG_NONE    0x00000000u

struct noos_boot_info {
    uint32_t boot_r0;
    uint32_t machine_type;
    uint32_t tags_address;
    uint32_t memory_start;
    uint32_t memory_size;
    const char *command_line;
    bool atags_valid;
};

void boot_info_init(struct noos_boot_info *info, uint32_t r0, uint32_t r1, uint32_t r2);
const struct noos_boot_info *boot_info_get(void);

#endif
