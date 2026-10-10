# noOS — Nokia Lumia 520 (RM-915)

Bare-metal ARMv7 bring-up target for noOS on the Nokia Lumia 520 / RM-915 (`fame_lta`).

## Current milestone

The kernel now:

- links at the verified legacy Fame kernel load address `0x80208000`;
- installs an ARM exception vector base;
- preserves the bootloader's `r0`, `r1`, and `r2` handoff;
- parses legacy ARM ATAGS from `r2`;
- records the first RAM bank and kernel command line when supplied;
- initializes a freestanding C runtime (`.bss`, stack, memory primitives);
- builds both a raw kernel and a legacy Android-style boot image;
- avoids unverified peripheral MMIO.

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
```

`noos.bin` is the raw payload for controlled chainloading experiments. `noos-boot.img` uses the known Fame legacy boot-image layout (4096-byte pages, base `0x80200000`, kernel at `0x80208000`, ATAGS at `0x80200100`).

## Do not flash blindly

The boot-image layout is sourced from an existing working Lumia 520 Linux port, but noOS is still early bring-up software. Prefer reversible/temporary boot paths and keep recovery firmware available.

See `docs/HARDWARE.md` and `docs/ROADMAP.md`.
