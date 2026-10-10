# RM-915 boot contract

## Legacy Fame layout

The current noOS image follows the known Lumia 520 / Fame legacy Android boot layout:

- image base: `0x80200000`
- kernel offset: `0x00008000`
- kernel entry/load: `0x80208000`
- ATAGS offset: `0x00000100`
- ATAGS address: `0x80200100`
- page size: 4096 bytes

## ARM32 register handoff

At entry noOS preserves:

- `r0` — boot protocol value
- `r1` — machine type
- `r2` — ATAGS/FDT pointer

The current parser expects legacy ATAGS. It recognizes:

- `ATAG_CORE`
- every `ATAG_MEM`
- `ATAG_CMDLINE`
- `ATAG_NONE`

Malformed tag sizes terminate parsing instead of walking unbounded memory.

## Build outputs

`noos.elf`
: linked debug image with symbols.

`noos.bin`
: raw binary payload.

`noos-boot.img`
: generated legacy Android v0-style boot container.

`tools/verify_bootimg.py` validates the generated container's magic, kernel address, tags address, page size, payload bounds, and empty ramdisk expectation.

## Current safety boundary

noOS still avoids unverified RM-915 peripheral MMIO. The boot contract and memory subsystem can advance independently while UART, clock, GSBI and pinmux details are being verified.
