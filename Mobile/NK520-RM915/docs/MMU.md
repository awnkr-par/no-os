# ARMv7 MMU planning

noOS now builds an ARMv7 short-descriptor L1 translation table, but **does not enable the MMU yet**.

## Current behavior

The kernel reserves a dedicated 16 KiB-aligned linker section:

```text
.mmu_tables
```

with exported symbols:

```text
__mmu_l1_start
__mmu_l1_end
```

The table contains 4096 first-level entries. Each entry covers 1 MiB.

During boot, noOS:

1. clears the L1 table;
2. identity-maps only sections intersecting bootloader-declared RAM banks;
3. uses section descriptors configured as full-access normal non-cached memory;
4. validates that every known RAM section resolves to the same physical section;
5. records the table address and mapped range in the diagnostic record;
6. leaves SCTLR.M untouched.

## Why MMU enablement is still disabled

Turning on the MMU before the full device map is known could immediately fault on:

- framebuffer accesses;
- UART/GSBI registers;
- interrupt-controller registers;
- timers;
- bootloader data still being inspected.

The current table builder exists so the mapping plan can be tested and inspected before any irreversible CPU-state transition.

## Next MMU work

Before enabling translation:

- add explicit device-memory mappings for verified MMIO ranges;
- assign access permissions and execute-never policy;
- decide cacheability for kernel RAM and framebuffer;
- program TTBR0/TTBCR/DACR;
- invalidate TLBs;
- sequence DSB/ISB barriers correctly;
- enable SCTLR.M only behind an explicit bring-up flag.
