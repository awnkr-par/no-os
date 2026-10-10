# RM-915 memory subsystem

## Boot memory map

noOS reads all legacy `ATAG_MEM` records supplied by the bootloader, up to `NOOS_MAX_MEMORY_BANKS`.

The kernel does **not** assume the Lumia's RAM is represented as one perfectly contiguous bank. Each valid ATAG memory record is preserved and fed to the physical memory manager.

## Physical memory manager

The PMM manages 4 KiB pages with two bitmaps:

- **used bitmap** — pages that are unavailable or currently allocated;
- **permanent bitmap** — pages that must never be returned by `pmm_free_page()`.

The current maximum managed span is 131,072 pages = 512 MiB, matching the shipping Lumia 520 RAM capacity.

At initialization all pages are treated as unavailable. Pages covered by valid ATAG RAM banks are then exposed as free.

The PMM permanently reserves:

- the complete linked noOS image;
- `.bss`;
- all mode-specific stacks;
- both PMM bitmaps themselves, because they live inside `.bss`;
- the page containing the bootloader ATAGS.

Pages allocated with `pmm_alloc_page()` may later be released with `pmm_free_page()`. Permanently reserved pages are rejected by `pmm_free_page()`.

## Kernel heap

The current heap is intentionally tiny and deterministic.

`kmalloc()`:

- aligns allocations to 8 bytes;
- obtains backing pages from the PMM;
- supports one allocation up to 4096 bytes;
- never frees individual allocations yet.

`kcalloc()` performs overflow checking and zero initialization.

This allocator is for bring-up structures, not userspace or long-lived production allocation.

## Next memory work

- explicit reservation API for future MMIO/DMA/device carve-outs;
- multiple-page heap allocations;
- slab or free-list kernel allocator;
- ARMv7 short-descriptor page tables;
- MMU/cache enablement only after the physical map is validated on hardware.
