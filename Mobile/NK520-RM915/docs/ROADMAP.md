# noOS RM-915 roadmap

## Milestone 0 — freestanding kernel foundation ✅

- ARMv7 entry point
- VBAR exception vector table
- stack and `.bss` setup
- C kernel entry
- memory primitives
- panic/idle path

## Milestone 1 — bootloader handoff ✅

- Fame kernel load address `0x80208000`
- preserve `r0`/`r1`/`r2`
- bounded legacy ATAG parser
- multiple `ATAG_MEM` banks
- command-line capture
- legacy Android `boot.img` generation and validation

## Milestone 2 — kernel diagnostics and memory ✅

- 4 KiB in-memory early console
- hex/decimal boot diagnostics
- persistent panic reason
- dedicated SVC/IRQ/FIQ/abort/undefined stacks
- register + ARM fault-status exception capture
- 512 MiB-capable bitmap PMM
- permanent page reservations
- allocation/free accounting
- page-backed kernel heap
- boot-time memory allocator self-tests
- CI architecture, entrypoint, symbol, artifact, and boot-image checks

## Milestone 3 — first observable hardware output

- identify exact Fame `ttyHSL0` controller resource
- verify GSBI protocol selection
- verify UART clocks/resets
- verify TLMM pinmux
- implement minimal UARTDM TX
- replay the complete early console over UART
- print live boot/fault diagnostics from the phone

## Milestone 4 — virtual memory and CPU state

- validate all device/reserved RAM regions
- ARMv7 short-descriptor translation tables
- identity-map early kernel + required MMIO
- domain/access permissions
- guarded MMU enablement
- I-cache/D-cache sequencing
- page-fault diagnostics against the new tables

## Milestone 5 — timer and interrupts

- identify KPSS timer resource
- interrupt controller
- real IRQ dispatch instead of fatal IRQ
- monotonic tick source
- sleep/delay primitives

## Milestone 6 — user-visible hardware

- framebuffer handoff or MDP display bring-up
- GPIO/button input
- PM8038/SSBI support
- USB debug transport

## Milestone 7 — storage and higher-level OS

- eMMC
- block layer
- filesystem
- scheduler/threads
- syscall ABI
- userspace loader
- userspace
