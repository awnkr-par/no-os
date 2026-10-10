#ifndef NOOS_STATUS_H
#define NOOS_STATUS_H

#include <stdint.h>

#define NOOS_STATUS_MAGIC 0x4E4F4F53u

enum noos_boot_stage {
    NOOS_STAGE_RESET = 1,
    NOOS_STAGE_C_ENTRY = 2,
    NOOS_STAGE_BOOT_INFO = 3,
    NOOS_STAGE_MEMORY = 4,
    NOOS_STAGE_HEAP = 5,
    NOOS_STAGE_SELFTEST = 6,
    NOOS_STAGE_READY = 7,
    NOOS_STAGE_PANIC = 0xFF
};

struct noos_boot_status {
    uint32_t magic;
    uint32_t version;
    uint32_t stage;
    uint32_t detail;
};

void boot_status_init(void);
void boot_status_set(enum noos_boot_stage stage, uint32_t detail);
const struct noos_boot_status *boot_status_get(void);

#endif
