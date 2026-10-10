#include <noos/boot.h>
#include <noos/memory.h>

struct atag_header {
    uint32_t size;
    uint32_t tag;
};

struct atag_mem32 {
    uint32_t size;
    uint32_t start;
};

struct atag {
    struct atag_header hdr;
    union {
        struct atag_mem32 mem;
        char cmdline[1];
    } u;
};

static struct noos_boot_info g_boot_info;

static bool atag_pointer_plausible(uint32_t address)
{
    return address >= 0x80000000u && address < 0xA0000000u && (address & 3u) == 0;
}

static void add_memory_bank(struct noos_boot_info *info, uint32_t start, uint32_t size)
{
    if (size == 0) {
        return;
    }

    if (info->memory_bank_count >= NOOS_MAX_MEMORY_BANKS) {
        info->memory_truncated = true;
        return;
    }

    struct noos_memory_bank *bank = &info->memory_banks[info->memory_bank_count++];
    bank->start = start;
    bank->size = size;

    if (UINT32_MAX - info->total_memory_bytes < size) {
        info->total_memory_bytes = UINT32_MAX;
    } else {
        info->total_memory_bytes += size;
    }
}

void boot_info_init(struct noos_boot_info *info, uint32_t r0, uint32_t r1, uint32_t r2)
{
    memset(info, 0, sizeof(*info));
    info->boot_r0 = r0;
    info->machine_type = r1;
    info->tags_address = r2;

    if (!atag_pointer_plausible(r2)) {
        g_boot_info = *info;
        return;
    }

    const struct atag *tag = (const struct atag *)(uintptr_t)r2;
    bool saw_core = false;

    for (uint32_t count = 0; count < 256; ++count) {
        if (tag->hdr.tag == NOOS_ATAG_NONE && tag->hdr.size == 0) {
            info->atags_valid = saw_core;
            break;
        }

        if (tag->hdr.size < 2 || tag->hdr.size > 0x10000u) {
            break;
        }

        switch (tag->hdr.tag) {
        case NOOS_ATAG_CORE:
            saw_core = true;
            break;
        case NOOS_ATAG_MEM:
            if (tag->hdr.size >= 4) {
                add_memory_bank(info, tag->u.mem.start, tag->u.mem.size);
            }
            break;
        case NOOS_ATAG_CMDLINE:
            info->command_line = tag->u.cmdline;
            break;
        default:
            break;
        }

        tag = (const struct atag *)((const uint32_t *)tag + tag->hdr.size);
    }

    g_boot_info = *info;
}

const struct noos_boot_info *boot_info_get(void)
{
    return &g_boot_info;
}
