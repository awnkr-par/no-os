# noOS — Nokia Lumia 520 (RM-915)

Bare-metal ARMv7 bring-up target for noOS on the Nokia Lumia 520 / RM-915 (`fame_lta`).

## Current status

This is no longer just an entry stub. The RM-915 target now has:

- Fame legacy kernel load/entry at `0x80208000`;
- VBAR-installed ARM exception vectors;
- dedicated SVC, IRQ, FIQ, abort, and undefined-mode stacks;
- fatal exception capture with r0-r12, LR, SPSR, DFSR/DFAR, and IFSR/IFAR;
- preserved ARM32 `r0`/`r1`/`r2` bootloader handoff;
- bounded legacy ATAG parsing;
- support for multiple `ATAG_MEM` RAM banks;
- command-line capture;
- a 4 KiB in-memory early console;
- persistent panic/fault diagnostic state;
- a 4 KiB-page bitmap physical memory manager;
- permanent reservation protection for the kernel image and ATAGS;
- page allocation and page freeing;
- a small page-backed, 8-byte-aligned kernel heap;
- boot-time memory/allocator self-tests;
- raw ELF/BIN and legacy Android `boot.img` outputs;
- boot-image verification tooling;
- GitHub Actions checks for ARM architecture, entrypoint, required kernel symbols, artifact validity, and disassembly generation.

Hardware-specific MMIO is still intentionally disabled until the exact RM-915 resources are verified.

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

The regular build also runs `tools/verify_bootimg.py` against the generated boot image.

## Bring-up path

`noos.bin` is the raw payload for controlled chainloading experiments.

`noos-boot.img` uses the known Fame legacy layout:

- page size: 4096
- base: `0x80200000`
- kernel: `0x80208000`
- ATAGS: `0x80200100`

## Current limitation

There is still no verified physical debug backend. The kernel can now produce a meaningful boot log and exception report in memory, but the exact RM-915 `ttyHSL0` UART controller, GSBI configuration, clocks, and pinmux still need verification before we write those registers.

## Documentation

- `docs/BOOT.md` — boot contract and image layout
- `docs/MEMORY.md` — RAM map, bitmap PMM, and heap
- `docs/EXCEPTIONS.md` — ARM exception stacks and crash capture
- `docs/HARDWARE.md` — verified hardware facts and safety boundary
- `docs/ROADMAP.md` — remaining bring-up milestones

## Do not flash blindly

noOS is early bare-metal bring-up software. Prefer reversible or temporary boot paths and keep recovery firmware available.
