# noOS RM-915 roadmap

## Milestone 0 — freestanding kernel foundation ✅

- ARMv7 entry point
- exception vector table
- stack and `.bss` setup
- C kernel entry
- memory primitives
- panic/idle path

## Milestone 1 — bootloader handoff ✅

- link at Fame kernel load address `0x80208000`
- preserve `r0`/`r1`/`r2`
- parse legacy ATAGS
- retain RAM-bank and command-line data
- package a legacy Android `boot.img`

## Milestone 2 — first observable output

- verify RM-915 ttyHSL0 controller resource
- verify GSBI mode, clock and pinmux
- implement minimal UARTDM TX
- print an early noOS banner and boot-register dump

## Milestone 3 — memory

- validate RAM map from ATAGS against known 512 MiB hardware
- reserve image/stack/boot-data regions
- page-frame allocator
- MMU page tables and guarded enablement

## Milestone 4 — timer and interrupts

- KPSS timer
- interrupt controller
- exception diagnostics

## Milestone 5 — user-visible hardware

- framebuffer/display handoff or MDP bring-up
- physical buttons through PM8038/SSBI
- USB debug transport

## Milestone 6 — storage and higher-level OS

- eMMC/SD
- filesystem
- scheduler/threads
- syscall boundary
- userspace
