#include <noos/boot.h>
#include <noos/fdt.h>
#include <noos/memory.h>

struct atag_header {
    uint32_t size;
    uint32_t tag;
};

struct atag_mem32 {
    uint32_t size;
    uint32_t start;
};

struct atag_videolfb {
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t line_length;
    uint32_t base;
    uint32_t size;
    uint8_t red_size;
    uint8_t red_pos;
    uint8_t green_size;
    uint8_t green_pos;
    uint8_t blue_size;
    uint8_t blue_pos;
    uint8_t reserved_size;
    uint8_t reserved_pos;
};

struct atag {
    struct atag_header hdr;
    union {
        struct atag_mem32 mem;
        struct atag_videolfb videolfb;
        char cmdline[1];
    } u;
};

static struct noos_boot_info g_boot_info;

static bool boot_pointer_plausible(uint32_t address)
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

static enum noos_pixel_format atag_video_format(const struct atag_videolfb *video)
{
    if (video->depth == 16 &&
        video->red_size == 5 && video->red_pos == 11 &&
        video->green_size == 6 && video->green_pos == 5 &&
        video->blue_size == 5 && video->blue_pos == 0) {
        return NOOS_PIXEL_RGB565;
    }

    if (video->depth == 32 &&
        video->red_size == 8 && video->red_pos == 16 &&
        video->green_size == 8 && video->green_pos == 8 &&
        video->blue_size == 8 && video->blue_pos == 0) {
        return video->reserved_size == 8 ? NOOS_PIXEL_ARGB8888 : NOOS_PIXEL_XRGB8888;
    }

    return NOOS_PIXEL_UNKNOWN;
}

static bool parse_atags(struct noos_boot_info *info, uint32_t address)
{
    if (!boot_pointer_plausible(address)) {
        return false;
    }

    const struct atag *tag = (const struct atag *)(uintptr_t)address;
    bool saw_core = false;

    for (uint32_t count = 0; count < 256; ++count) {
        if (tag->hdr.tag == NOOS_ATAG_NONE && tag->hdr.size == 0) {
            return saw_core;
        }

        if (tag->hdr.size < 2 || tag->hdr.size > 0x10000u) {
            return false;
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
        case NOOS_ATAG_VIDEOLFB:
            if (tag->hdr.size >= 8) {
                const enum noos_pixel_format format = atag_video_format(&tag->u.videolfb);
                if (format != NOOS_PIXEL_UNKNOWN) {
                    info->framebuffer.address = tag->u.videolfb.base;
                    info->framebuffer.size = tag->u.videolfb.size;
                    info->framebuffer.width = tag->u.videolfb.width;
                    info->framebuffer.height = tag->u.videolfb.height;
                    info->framebuffer.stride = tag->u.videolfb.line_length;
                    info->framebuffer.format = format;
                    info->framebuffer.valid = true;
                }
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

    return false;
}

void boot_info_init(struct noos_boot_info *info, uint32_t r0, uint32_t r1, uint32_t r2)
{
    memset(info, 0, sizeof(*info));
    info->boot_r0 = r0;
    info->machine_type = r1;
    info->boot_data_address = r2;

    if (fdt_probe(r2)) {
        info->protocol = NOOS_BOOT_DTB;
        info->boot_data_valid = fdt_parse_boot_info(info, r2);
    } else {
        info->protocol = NOOS_BOOT_ATAGS;
        info->boot_data_valid = parse_atags(info, r2);
    }

    if (!info->boot_data_valid) {
        info->protocol = NOOS_BOOT_UNKNOWN;
    }

    g_boot_info = *info;
}

const struct noos_boot_info *boot_info_get(void)
{
    return &g_boot_info;
}
