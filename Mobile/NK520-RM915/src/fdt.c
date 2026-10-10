#include <noos/fdt.h>
#include <noos/memory.h>
#include <stddef.h>
#include <stdint.h>

#define FDT_BEGIN_NODE 0x1u
#define FDT_END_NODE   0x2u
#define FDT_PROP       0x3u
#define FDT_NOP        0x4u
#define FDT_END        0x9u
#define FDT_MAX_DEPTH  24u

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

struct node_state {
    bool chosen;
    bool memory_name;
    bool memory_type;
    bool simplefb;
    uint32_t reg_address;
    uint32_t reg_size;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    enum noos_pixel_format format;
};

static uint32_t be32(const void *ptr)
{
    const uint8_t *p = (const uint8_t *)ptr;
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}

static bool string_equal(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (*a++ != *b++) {
            return false;
        }
    }
    return *a == *b;
}

static bool string_starts_with(const char *text, const char *prefix)
{
    while (*prefix != '\0') {
        if (*text++ != *prefix++) {
            return false;
        }
    }
    return true;
}

static uintptr_t align4(uintptr_t value)
{
    return (value + 3u) & ~(uintptr_t)3u;
}

static bool read_cells_u32(const uint8_t *data, uint32_t cells, uint32_t *value)
{
    if (cells == 0 || cells > 2) {
        return false;
    }

    if (cells == 1) {
        *value = be32(data);
        return true;
    }

    if (be32(data) != 0) {
        return false;
    }

    *value = be32(data + 4);
    return true;
}

static enum noos_pixel_format parse_format(const char *format)
{
    if (string_equal(format, "x8r8g8b8") || string_equal(format, "x8b8g8r8")) {
        return NOOS_PIXEL_XRGB8888;
    }
    if (string_equal(format, "a8r8g8b8") || string_equal(format, "a8b8g8r8")) {
        return NOOS_PIXEL_ARGB8888;
    }
    if (string_equal(format, "r5g6b5")) {
        return NOOS_PIXEL_RGB565;
    }
    return NOOS_PIXEL_UNKNOWN;
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

    info->memory_banks[info->memory_bank_count].start = start;
    info->memory_banks[info->memory_bank_count].size = size;
    info->memory_bank_count++;

    if (UINT32_MAX - info->total_memory_bytes < size) {
        info->total_memory_bytes = UINT32_MAX;
    } else {
        info->total_memory_bytes += size;
    }
}

bool fdt_probe(uint32_t address)
{
    if ((address & 7u) != 0 || address < 0x80000000u || address >= 0xA0000000u) {
        return false;
    }

    return be32((const void *)(uintptr_t)address) == NOOS_FDT_MAGIC;
}

bool fdt_parse_boot_info(struct noos_boot_info *info, uint32_t address)
{
    if (info == 0 || !fdt_probe(address)) {
        return false;
    }

    const uint8_t *base = (const uint8_t *)(uintptr_t)address;
    const struct fdt_header *header = (const struct fdt_header *)base;
    const uint32_t total = be32(&header->totalsize);
    const uint32_t struct_off = be32(&header->off_dt_struct);
    const uint32_t strings_off = be32(&header->off_dt_strings);
    const uint32_t struct_size = be32(&header->size_dt_struct);
    const uint32_t strings_size = be32(&header->size_dt_strings);

    if (total < sizeof(*header) ||
        struct_off >= total || strings_off >= total ||
        struct_size > total - struct_off ||
        strings_size > total - strings_off) {
        return false;
    }

    const uint8_t *struct_begin = base + struct_off;
    const uint8_t *struct_end = struct_begin + struct_size;
    const char *strings = (const char *)(base + strings_off);

    struct node_state states[FDT_MAX_DEPTH];
    memset(states, 0, sizeof(states));

    uint32_t depth = 0;
    uint32_t address_cells = 1;
    uint32_t size_cells = 1;
    const uint8_t *cursor = struct_begin;

    while (cursor + 4 <= struct_end) {
        const uint32_t token = be32(cursor);
        cursor += 4;

        if (token == FDT_BEGIN_NODE) {
            if (depth >= FDT_MAX_DEPTH) {
                return false;
            }

            struct node_state *node = &states[depth];
            memset(node, 0, sizeof(*node));

            const char *name = (const char *)cursor;
            const uint8_t *name_scan = cursor;
            while (name_scan < struct_end && *name_scan != '\0') {
                name_scan++;
            }
            if (name_scan >= struct_end) {
                return false;
            }

            node->chosen = string_equal(name, "chosen");
            node->memory_name = string_equal(name, "memory") ||
                                string_starts_with(name, "memory@");

            cursor = (const uint8_t *)align4((uintptr_t)(name_scan + 1));
            if (cursor > struct_end) {
                return false;
            }

            depth++;
            continue;
        }

        if (token == FDT_END_NODE) {
            if (depth == 0) {
                return false;
            }

            struct node_state *node = &states[depth - 1];

            if ((node->memory_name || node->memory_type) &&
                node->reg_size != 0) {
                add_memory_bank(info, node->reg_address, node->reg_size);
            }

            if (node->simplefb &&
                node->reg_address != 0 &&
                node->width != 0 &&
                node->height != 0 &&
                node->stride != 0 &&
                node->format != NOOS_PIXEL_UNKNOWN) {
                info->framebuffer.address = node->reg_address;
                info->framebuffer.size = node->reg_size;
                info->framebuffer.width = node->width;
                info->framebuffer.height = node->height;
                info->framebuffer.stride = node->stride;
                info->framebuffer.format = node->format;
                info->framebuffer.valid = true;
            }

            depth--;
            continue;
        }

        if (token == FDT_PROP) {
            if (depth == 0 || cursor + 8 > struct_end) {
                return false;
            }

            const uint32_t len = be32(cursor);
            const uint32_t nameoff = be32(cursor + 4);
            cursor += 8;

            if (len > (uint32_t)(struct_end - cursor) || nameoff >= strings_size) {
                return false;
            }

            const char *prop_name = strings + nameoff;
            const uint8_t *value = cursor;
            struct node_state *node = &states[depth - 1];

            if (depth == 1 && string_equal(prop_name, "#address-cells") && len >= 4) {
                address_cells = be32(value);
            } else if (depth == 1 && string_equal(prop_name, "#size-cells") && len >= 4) {
                size_cells = be32(value);
            } else if (node->chosen && string_equal(prop_name, "bootargs") && len != 0) {
                info->command_line = (const char *)value;
            } else if (string_equal(prop_name, "device_type") && len >= 7) {
                node->memory_type = string_equal((const char *)value, "memory");
            } else if (string_equal(prop_name, "compatible") && len != 0) {
                const char *compat = (const char *)value;
                const uint8_t *limit = value + len;
                while ((const uint8_t *)compat < limit && *compat != '\0') {
                    if (string_equal(compat, "simple-framebuffer")) {
                        node->simplefb = true;
                        break;
                    }

                    const char *p = compat;
                    while ((const uint8_t *)p < limit && *p != '\0') {
                        p++;
                    }
                    if ((const uint8_t *)p >= limit) {
                        break;
                    }
                    compat = p + 1;
                }
            } else if (string_equal(prop_name, "reg")) {
                const uint32_t cells = address_cells + size_cells;
                if (cells != 0 && len >= cells * 4u) {
                    uint32_t reg_address;
                    uint32_t reg_size;
                    if (read_cells_u32(value, address_cells, &reg_address) &&
                        read_cells_u32(value + address_cells * 4u, size_cells, &reg_size)) {
                        node->reg_address = reg_address;
                        node->reg_size = reg_size;
                    }
                }
            } else if (string_equal(prop_name, "width") && len >= 4) {
                node->width = be32(value);
            } else if (string_equal(prop_name, "height") && len >= 4) {
                node->height = be32(value);
            } else if (string_equal(prop_name, "stride") && len >= 4) {
                node->stride = be32(value);
            } else if (string_equal(prop_name, "format") && len != 0) {
                node->format = parse_format((const char *)value);
            }

            cursor = (const uint8_t *)align4((uintptr_t)(cursor + len));
            if (cursor > struct_end) {
                return false;
            }
            continue;
        }

        if (token == FDT_NOP) {
            continue;
        }

        if (token == FDT_END) {
            return depth == 0;
        }

        return false;
    }

    return false;
}
