#include <noos/mmu.h>
#include <noos/memory.h>
#include <stdint.h>

#define MMU_DESC_SECTION      0x2u
#define MMU_SECTION_AP_RW     (3u << 10)
#define MMU_SECTION_TEX_NC    (1u << 12)

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

static bool descriptor_is_section(uint32_t descriptor)
{
    return (descriptor & 0x3u) == MMU_DESC_SECTION;
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
            g_l1_table[index] =
                normal_noncached_descriptor(index * NOOS_MMU_SECTION_SIZE);
            g_stats.mapped_sections++;
            g_stats.ram_sections++;

            if (g_stats.mapped_sections == 1) {
                g_stats.first_mapped_section = index;
            }
            g_stats.last_mapped_section = index;
        }
    }
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

    return true;
}

uint32_t mmu_l1_descriptor(uint32_t virtual_address)
{
    return g_l1_table[section_index(virtual_address)];
}

const struct mmu_stats *mmu_get_stats(void)
{
    return &g_stats;
}
