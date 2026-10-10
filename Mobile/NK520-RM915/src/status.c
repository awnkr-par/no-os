#include <noos/status.h>

static volatile struct noos_boot_status g_status;

void boot_status_init(void)
{
    g_status.magic = NOOS_STATUS_MAGIC;
    g_status.version = 1;
    g_status.stage = NOOS_STAGE_C_ENTRY;
    g_status.detail = 0;
}

void boot_status_set(enum noos_boot_stage stage, uint32_t detail)
{
    g_status.stage = (uint32_t)stage;
    g_status.detail = detail;
    __asm__ volatile ("" ::: "memory");
}

const struct noos_boot_status *boot_status_get(void)
{
    return (const struct noos_boot_status *)&g_status;
}
