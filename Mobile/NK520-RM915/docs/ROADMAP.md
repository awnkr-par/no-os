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
- DTB memreserve parsing
- `/reserved-memory` parsing
- initrd detection/reservation
- full DTB blob reservation
- legacy Android `boot.img` generation and validation

## Milestone 2 — kernel diagnostics and memory ✅

- 4 KiB in-memory early console
- hex/decimal boot diagnostics
- boot-stage breadcrumb structure
- persistent panic reason
- dedicated SVC/IRQ/FIQ/abort/undefined stacks
- register + ARM fault-status exception capture
- 512 MiB-capable bitmap PMM
- permanent kernel/boot-data/framebuffer/initrd/DTB reservations
- allocation/free accounting
- page-backed kernel heap
- boot-time memory allocator self-tests
- double-free rejection test
- reserved-range permanence test
- checksummed `.noos_diag` block + decoder
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

## Milestone 4 — physical debug transport 🚧

Implemented / source-verified:

- Fame/Lumia stable LK GSBI5 selection
- GSBI base `0x16400000`
- UARTDM base `0x16440000`
- RX GPIO23 / TX GPIO22 function 1
- bounded legacy UARTDM TX path
- opt-in LK/lk2nd UART handoff backend
- live early-console mirroring over UART
- UART health/timeout diagnostics
- full clock/TLMM init plan with compile-time and boot-time validation
- separate safe/default and UART-enabled CI artifacts

Remaining:

- validate UART handoff on physical RM-915
- port full cold-init clock branch/root/halt sequencing
- enable full initialization only behind an explicit experimental flag

## Milestone 5 — virtual memory and CPU state 🚧

Implemented:

- 16 KiB-aligned ARMv7 short-descriptor L1 table
- identity-map all declared RAM with 1 MiB sections
- validate every generated identity mapping
- record MMU plan in diagnostics
- keep SCTLR.M untouched

Remaining:

- validate all device/reserved RAM regions
- identity-map required MMIO
- verified TLMM, clock-controller, and GSBI5/UARTDM sections are already mapped as shareable Device + XN
- domain/access permissions
- guarded MMU enablement after device mappings exist
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
