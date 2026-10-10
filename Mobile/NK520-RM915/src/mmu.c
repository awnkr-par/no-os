#include <noos/mmu.h>
#include <noos/memory.h>
#include <stdint.h>

#define MMU_DESC_SECTION        0x2u
#define MMU_SECTION_B           (1u << 2)
#define MMU_SECTION_XN          (1u << 4)
#define MMU_SECTION_AP_RW       (3u << 10)
#define MMU_SECTION_TEX_NC      (1u << 12)
#define MMU_SECTION_SHAREABLE   (1u << 16)

#define RM915_TLMM_SECTION      0x00800000u
#define RM915_CLK_SECTION       0x00900000u
#define RM915_GSBI5_SECTION     0x16400000u

__attribute__((section(".mmu_tables"), used, aligned(16384)))
static uint32_t g_l1_table[NOOS_MMU_L1_ENTRIES];

static struct mmu_stats g_stats;

static uint32_t section_index(uint32_t address)
{
    return address >> 20;
}

static uint32_t section_base(uint32_t address)
{
    return address & 0xFFF00000u;
}

static uint32_t normal_noncached_descriptor(uint32_t physical)
{
    return section_base(physical) |
           MMU_DESC_SECTION |
           MMU_SECTION_AP_RW |
           MMU_SECTION_TEX_NC;
}

static uint32_t device_descriptor(uint32_t physical)
{
    /*
     * ARMv7 short-descriptor section:
     * TEX=0, C=0, B=1 => Device memory
     * S=1             => shareable
     * XN=1            => never execute MMIO
     * AP=0b11         => privileged/user RW for early bring-up
     */
    return section_base(physical) |
           MMU_DESC_SECTION |
           MMU_SECTION_AP_RW |
           MMU_SECTION_B |
           MMU_SECTION_SHAREABLE |
           MMU_SECTION_XN;
}

static bool descriptor_is_section(uint32_t descriptor)
{
    return (descriptor & 0x3u) == MMU_DESC_SECTION;
}

bool mmu_descriptor_is_device(uint32_t descriptor)
{
    const uint32_t tex = (descriptor >> 12) & 0x7u;
    const bool cacheable = (descriptor & (1u << 3)) != 0;
    const bool bufferable = (descriptor & MMU_SECTION_B) != 0;
    const bool shareable = (descriptor & MMU_SECTION_SHAREABLE) != 0;
    const bool xn = (descriptor & MMU_SECTION_XN) != 0;

    return descriptor_is_section(descriptor) &&
           tex == 0 &&
           !cacheable &&
           bufferable &&
           shareable &&
           xn;
}

static void note_mapped_index(uint32_t index)
{
    if (g_stats.mapped_sections == 0) {
        g_stats.first_mapped_section = index;
    }
    if (index < g_stats.first_mapped_section) {
        g_stats.first_mapped_section = index;
    }
    if (index > g_stats.last_mapped_section) {
        g_stats.last_mapped_section = index;
    }
}

static void map_ram_range(uint32_t start, uint32_t size)
{
    if (size == 0 || start + size < start) {
        return;
    }

    const uint32_t first = section_index(start);
    const uint32_t last = section_index(start + size - 1u);

    for (uint32_t index = first;
         index <= last && index < NOOS_MMU_L1_ENTRIES;
         ++index) {
        if (!descriptor_is_section(g_l1_table[index])) {
            note_mapped_index(index);
            g_l1_table[index] =
                normal_noncached_descriptor(index * NOOS_MMU_SECTION_SIZE);
            g_stats.mapped_sections++;
            g_stats.ram_sections++;
        }
    }
}

static void map_device_section(uint32_t address)
{
    const uint32_t index = section_index(address);
    if (index >= NOOS_MMU_L1_ENTRIES) {
        return;
    }

    if (!descriptor_is_section(g_l1_table[index])) {
        note_mapped_index(index);
        g_l1_table[index] =
            device_descriptor(index * NOOS_MMU_SECTION_SIZE);
        g_stats.mapped_sections++;
        g_stats.device_sections++;
    }
}

static bool validate_device_section(uint32_t address)
{
    const uint32_t descriptor = g_l1_table[section_index(address)];
    return section_base(descriptor) == section_base(address) &&
           mmu_descriptor_is_device(descriptor);
}

bool mmu_prepare_identity(const struct noos_boot_info *boot)
{
    memset(g_l1_table, 0, sizeof(g_l1_table));
    memset(&g_stats, 0, sizeof(g_stats));

    g_stats.table_address = (uint32_t)(uintptr_t)g_l1_table;

    if (boot == 0 || boot->memory_bank_count == 0 ||
        (g_stats.table_address & 0x3FFFu) != 0) {
        return false;
    }

    for (uint32_t i = 0; i < boot->memory_bank_count; ++i) {
        map_ram_range(boot->memory_banks[i].start,
                      boot->memory_banks[i].size);
    }

    /*
     * Verified RM-915/L520 stable LK paths needed by the current debug
     * transport and future full UART takeover.
     */
    map_device_section(RM915_TLMM_SECTION);
    map_device_section(RM915_CLK_SECTION);
    map_device_section(RM915_GSBI5_SECTION);

    g_stats.prepared = g_stats.mapped_sections != 0;
    if (!g_stats.prepared) {
        return false;
    }

    g_stats.validated = mmu_validate_identity(boot);
    return g_stats.validated;
}

bool mmu_validate_identity(const struct noos_boot_info *boot)
{
    if (boot == 0) {
        return false;
    }

    for (uint32_t i = 0; i < boot->memory_bank_count; ++i) {
        const struct noos_memory_bank *bank = &boot->memory_banks[i];

        if (bank->size == 0 || bank->start + bank->size < bank->start) {
            return false;
        }

        const uint32_t first = section_index(bank->start);
        const uint32_t last = section_index(bank->start + bank->size - 1u);

        for (uint32_t index = first;
             index <= last && index < NOOS_MMU_L1_ENTRIES;
             ++index) {
            const uint32_t descriptor = g_l1_table[index];
            if (!descriptor_is_section(descriptor) ||
                section_base(descriptor) != index * NOOS_MMU_SECTION_SIZE) {
                return false;
            }
        }
    }

    return validate_device_section(RM915_TLMM_SECTION) &&
           validate_device_section(RM915_CLK_SECTION) &&
           validate_device_section(RM915_GSBI5_SECTION);
}

uint32_t mmu_l1_descriptor(uint32_t virtual_address)
{
    return g_l1_table[section_index(virtual_address)];
}

const struct mmu_stats *mmu_get_stats(void)
{
    return &g_stats;
}
