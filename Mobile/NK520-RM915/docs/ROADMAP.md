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
- detect ATAGS vs DTB
- bounded legacy ATAG parser
- multiple `ATAG_MEM` banks
- bounded FDT walker
- DTB RAM and `/chosen/bootargs` parsing
- legacy Android `boot.img` generation and validation

## Milestone 2 — kernel diagnostics and memory ✅

- 4 KiB in-memory early console
- hex/decimal boot diagnostics
- boot-stage breadcrumb structure
- persistent panic reason
- dedicated SVC/IRQ/FIQ/abort/undefined stacks
- register + ARM fault-status exception capture
- 512 MiB-capable bitmap PMM
- permanent kernel/boot-data/framebuffer reservations
- allocation/free accounting
- page-backed kernel heap
- boot-time memory allocator self-tests
- CI architecture, entrypoint, symbol, artifact, and boot-image checks

## Milestone 3 — first observable hardware output ✅/TEST NEEDED

Implemented:

- parse DTB `simple-framebuffer`
- support XRGB8888, ARGB8888, and RGB565
- reserve framebuffer memory from PMM
- primitive text renderer
- noOS proof-of-life splash
- panic/failure splash

Needs real Lumia validation:

- boot through lk2nd with `lk2nd.pass-simplefb=xrgb8888`
- determine whether Fame requires `autorefresh`
- confirm stride/pixel format and panel persistence on hardware

## Milestone 4 — physical debug transport

- identify exact Fame `ttyHSL0` controller resource
- verify GSBI protocol selection
- verify UART clocks/resets
- verify TLMM pinmux
- implement minimal UARTDM TX
- replay the complete early console over UART

## Milestone 5 — virtual memory and CPU state

- validate all device/reserved RAM regions
- ARMv7 short-descriptor translation tables
- identity-map early kernel + required MMIO
- domain/access permissions
- guarded MMU enablement
- I-cache/D-cache sequencing
- page-fault diagnostics against the new tables

## Milestone 6 — timer and interrupts

- identify KPSS timer resource
- interrupt controller
- real IRQ dispatch instead of fatal IRQ
- monotonic tick source
- sleep/delay primitives

## Milestone 7 — native hardware drivers

- MDP/display controller bring-up
- GPIO/button input
- PM8038/SSBI support
- USB debug transport
- eMMC/block layer

## Milestone 8 — higher-level OS

- filesystem
- scheduler/threads
- syscall ABI
- userspace loader
- userspace
