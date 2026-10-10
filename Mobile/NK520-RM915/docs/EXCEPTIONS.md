# ARMv7 exception handling

noOS installs its vector table through ARMv7 VBAR during early reset.

## Dedicated mode stacks

The linker reserves separate banked stacks for:

- SVC: 16 KiB
- IRQ: 4 KiB
- FIQ: 4 KiB
- Abort: 4 KiB
- Undefined: 4 KiB

Reset code switches through the CPU modes and initializes each banked stack pointer before entering C.

## Captured state

Every currently wired exception is treated as fatal during early bring-up.

The assembly vector stub saves:

- r0-r12
- exception-mode LR
- SPSR

The C exception handler additionally records:

- reconstructed fault PC
- DFSR
- DFAR
- IFSR
- IFAR
- exception type

The state is retained in `struct noos_fault_state` and the early console receives a compact crash report before the panic path halts the processor.

## PC correction

ARM exception LR values differ by exception class. noOS currently compensates:

- data abort: LR - 8
- prefetch abort: LR - 4
- IRQ/FIQ: LR - 4
- undefined/SVC: LR unchanged

This is sufficient for bring-up diagnostics. IRQ/FIQ will become non-fatal only after an interrupt controller driver exists.

## Why this exists before UART

The early console is memory-backed. Even if no physical debug output exists yet, a debugger, RAM dump, later UART backend, or framebuffer backend can replay the crash information.
