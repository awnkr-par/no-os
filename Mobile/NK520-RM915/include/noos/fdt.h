#ifndef NOOS_FDT_H
#define NOOS_FDT_H

#include <noos/boot.h>
#include <stdbool.h>
#include <stdint.h>

#define NOOS_FDT_MAGIC 0xD00DFEEDu

bool fdt_probe(uint32_t address);
bool fdt_parse_boot_info(struct noos_boot_info *info, uint32_t address);

#endif
