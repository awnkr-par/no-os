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

## Milestone 1.5 — kernel diagnostics and memory ✅

- 4 KiB early in-memory console buffer
- hexadecimal and decimal boot diagnostics
- persistent panic reason
- first-bank 4 KiB physical page allocator
- allocator begins after the linked kernel image/stack
- automated RM-915 CI build on pushes to `main`

The early console intentionally buffers output in RAM until a verified UART or framebuffer backend exists.

## Milestone 2 — first observable hardware output

- verify RM-915 ttyHSL0 controller resource
- verify GSBI mode, clock and pinmux
- implement minimal UARTDM TX
- replay the early console over UART
- print boot registers, ATAGS state, RAM region and PMM statistics

## Milestone 3 — memory protection

- validate the full RAM map and reserved regions
- support multiple memory banks if Fame exposes them
- reserve bootloader/ATAGS/device regions explicitly
- MMU page tables
- guarded MMU + cache enablement

## Milestone 4 — timer and interrupts

- KPSS timer
- interrupt controller
- dedicated exception-mode stacks
- exception diagnostics and fault register dumps

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
