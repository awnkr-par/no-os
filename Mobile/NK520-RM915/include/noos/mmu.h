#ifndef NOOS_MMU_H
#define NOOS_MMU_H

#include <noos/boot.h>
#include <stdbool.h>
#include <stdint.h>

#define NOOS_MMU_L1_ENTRIES 4096u
#define NOOS_MMU_SECTION_SIZE 0x00100000u

struct mmu_stats {
    uint32_t table_address;
    uint32_t mapped_sections;
    uint32_t ram_sections;
    uint32_t device_sections;
    uint32_t first_mapped_section;
    uint32_t last_mapped_section;
    bool prepared;
    bool validated;
};

bool mmu_prepare_identity(const struct noos_boot_info *boot);
bool mmu_validate_identity(const struct noos_boot_info *boot);
uint32_t mmu_l1_descriptor(uint32_t virtual_address);
bool mmu_descriptor_is_device(uint32_t descriptor);
const struct mmu_stats *mmu_get_stats(void);

#endif
