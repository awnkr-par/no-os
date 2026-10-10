# RM-915 timer and interrupt-controller plan

The current timer/GIC support is **validation-only**. noOS records and self-tests the Lumia/MSM8960-family register plan but does not take ownership of the timer or interrupt controller yet.

## Verified source path

Android4Lumia `bootloader_msm8227`, branch `L520_stable`, uses `platform/msm8960` for MSM8227.

`platform/msm8960/include/platform/iomap.h` defines:

```text
GIC distributor  0x02000000
GIC CPU iface    0x02002000
timer base       0x0200A000
GPT base         0x0200A004
DGT base         0x0200A024
```

Derived timer registers encoded in noOS:

| Register | Address |
| --- | ---: |
| GPT count | `0x0200A008` |
| GPT enable | `0x0200A00C` |
| GPT clear | `0x0200A010` |
| DGT count | `0x0200A028` |
| DGT enable | `0x0200A02C` |
| DGT clear | `0x0200A030` |
| DGT clock control | `0x0200A034` |

`platform/msm8960/platform.c` initializes the DGT by disabling it, programming clock divider value `3`, and records a tick rate of **6,750,000 Hz** (`27 MHz / 4`).

## What noOS does today

- compile-time asserts every derived register address;
- boot self-tests the complete timer/GIC plan;
- dry-run MMU maps the whole `0x02000000` section as shareable Device + XN;
- does **not** write DGT/GPT/GIC registers;
- does **not** enable interrupts;
- keeps IRQ/FIQ vectors fatal while controller bring-up remains incomplete.

## Next safe step

A future opt-in timer-owning mode can:

1. program `DGT_CLK_CTL = 3`;
2. clear and start the DGT;
3. use its 6.75 MHz counter for bounded hardware timeouts;
4. only after that replace loop-count UART timeouts;
5. later initialize QGIC and convert IRQ from fatal diagnostics to dispatch.

This should remain behind an explicit bring-up flag until verified on physical RM-915 hardware.
