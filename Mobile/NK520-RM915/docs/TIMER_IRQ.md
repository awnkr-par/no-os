# RM-915 live DGT scheduler tick

The live timer path is experimental and opt-in.

Enable with:

```text
noos.timer-irq=1
```

## Source-backed constants

```text
DGT base       0x0200A024
match          0x0200A024
count          0x0200A028
enable         0x0200A02C
clear          0x0200A030
clock control  0x0200A034
PPI IRQ        17
clock          6,750,000 Hz
```

The clock divider is programmed to value `3`, matching the Lumia/MSM8960-family reference path.

## Current period

noOS currently requests a 100 Hz kernel tick:

```text
6,750,000 / 100 = 67,500 timer counts
```

The DGT match value is therefore `67500`, with enable + clear-on-match set so the counter produces a periodic match.

## Interrupt flow

```text
DGT match
  -> QGIC PPI 17
  -> ARM IRQ vector
  -> exception_irq_handle()
  -> qgic_handle_one()
  -> irq_dispatch(17)
  -> rm915_timer_irq_handler()
  -> scheduler_tick()
  -> QGIC EOI
  -> exception return
```

This currently advances scheduler time and wakes sleeping task records. It does **not** yet save/restore task CPU contexts or preempt the currently executing kernel flow.
