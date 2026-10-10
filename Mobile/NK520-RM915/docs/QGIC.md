# RM-915 QGIC interrupt path

The RM-915/MSM8227 bring-up path uses the MSM8960-family QGIC layout already used by the Lumia bootloader/kernel sources.

## Physical bases

```text
QGIC distributor  0x02000000
QGIC CPU iface    0x02002000
```

CPU interface offsets used by noOS:

```text
CTRL       +0x00
PRIMASK    +0x04
INTACK     +0x0C
EOI        +0x10
```

Distributor offsets used by noOS:

```text
CTRL            +0x000
TYPER           +0x004
ENABLE_SET      +0x100
ENABLE_CLEAR    +0x180
PENDING_CLEAR   +0x280
PRIORITY        +0x400
CONFIG          +0xC00
```

## Ownership sequence

The live path is disabled unless the kernel command line contains:

```text
noos.timer-irq=1
```

When enabled, noOS:

1. keeps ARM IRQs masked;
2. initializes the generic IRQ registry;
3. disables the QGIC distributor while taking ownership;
4. disables/clears global SPIs;
5. resets banked PPI enables while leaving SGIs available;
6. configures CPU priority mask;
7. enables QGIC CPU/distributor interfaces;
8. configures DGT PPI 17 as edge-rising;
9. clears any pending DGT PPI state;
10. enables the DGT timer;
11. executes `cpsie i` only after all prior steps succeed.

Unhandled hardware IRQs are EOI'd and then escalated to the existing fatal exception path.

## ARM IRQ return

The IRQ vector saves `r0-r12,lr`, calls the C IRQ handler, restores registers, and returns with:

```asm
subs pc, lr, #4
```

which restores CPSR from SPSR as part of the exception return.
