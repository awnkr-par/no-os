# noOS — Nokia Lumia 520 (RM-915)

Bare-metal ARMv7 bring-up target for noOS on the Nokia Lumia 520 / RM-915 (`fame_lta`).

## Current status

The RM-915 target now has:

- Fame legacy kernel load/entry at `0x80208000`;
- VBAR-installed ARM exception vectors;
- dedicated SVC, IRQ, FIQ, abort, and undefined-mode stacks;
- fatal exception capture with r0-r12, LR, SPSR, DFSR/DFAR, and IFSR/IFAR;
- preserved ARM32 `r0`/`r1`/`r2` bootloader handoff;
- automatic detection of legacy ATAGS **or** DTB in `r2`;
- bounded ATAG parsing with multiple `ATAG_MEM` banks;
- bounded FDT parsing for RAM, `/chosen/bootargs`, `simple-framebuffer`, memreserve entries, `/reserved-memory`, and initrd ranges;
- a bootloader-framebuffer proof-of-life renderer;
- a 4 KiB in-memory early console;
- persistent panic/fault diagnostics;
- boot-stage breadcrumbs with a `NOOS` magic value;
- a 4 KiB-page bitmap physical memory manager;
- permanent reservation protection for kernel, full boot-data blobs, framebuffer, initrd, DTB memreserve entries, and `/reserved-memory`;
- page allocation and page freeing;
- a small page-backed, 8-byte-aligned kernel heap;
- boot-time memory/allocator self-tests including double-free and reservation protection;
- a dedicated checksummed `.noos_diag` crash/boot record with a host-side decoder;
- a 16 KiB-aligned ARMv7 L1 translation table builder that validates identity mappings without enabling the MMU;
- raw ELF/BIN and legacy Android `boot.img` outputs;
- boot-image verification tooling;
- GitHub Actions checks for ARM architecture, entrypoint, core symbols, artifact validity, and disassembly generation.

Hardware-specific Qualcomm MMIO is still intentionally disabled until the exact RM-915 resources are verified.

## Build

GNU Arm Embedded toolchain:

```sh
make -f GNUmakefile
```

LLVM/Clang:

```sh
make -f GNUmakefile LLVM=1
```

Outputs:

```text
build/noos.elf
build/noos.bin
build/noos-boot.img
build/noos.map
build/noos.lst   # after make disasm / CI
```

The regular build also runs `tools/verify_bootimg.py`.

By default the generated boot image embeds:

```text
lk2nd.pass-simplefb=xrgb8888
```

Override it when testing command-mode refresh behavior:

```sh
make -f GNUmakefile BOOT_CMDLINE='lk2nd.pass-simplefb=xrgb8888,autorefresh'
```

## Bring-up path

`noos.bin` is the raw payload for controlled chainloading experiments.

`noos-boot.img` uses the known Fame legacy layout:

- page size: 4096
- base: `0x80200000`
- kernel: `0x80208000`
- legacy ATAGS location: `0x80200100`

When lk2nd provides a DTB with `simple-framebuffer`, noOS can reuse that framebuffer and draw:

```text
NOOS
RM-915
SELFTEST PASS
```

without programming the Qualcomm display controller.

## Current limitation

UART is still not implemented because the exact RM-915 `ttyHSL0` UART controller, GSBI configuration, clocks, and TLMM pinmux still need verification.

The first-phone test should therefore prefer lk2nd's simple-framebuffer DTB handoff.

## Documentation

- `docs/BOOT.md` — boot contract and image layout
- `docs/CHAINLOAD.md` — intended temporary lk2nd/Fastboot test path
- `docs/DISPLAY.md` — simple-framebuffer proof-of-life output
- `docs/STATUS.md` — in-memory boot-stage breadcrumbs
- `docs/DIAGNOSTICS.md` — persistent diagnostic record + decoder
- `docs/MMU.md` — dry-run ARMv7 translation-table planning
- `docs/MEMORY.md` — RAM map, bitmap PMM, and heap
- `docs/EXCEPTIONS.md` — ARM exception stacks and crash capture
- `docs/HARDWARE.md` — verified hardware facts and safety boundary
- `docs/ROADMAP.md` — remaining bring-up milestones

## Do not flash blindly

noOS is early bare-metal bring-up software. Prefer reversible or temporary boot paths and keep recovery firmware available.
