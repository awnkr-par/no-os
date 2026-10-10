# Generic IRQ core

noOS now has an OS-side interrupt registry, independent of Qualcomm QGIC ownership.

## Implemented

- 256 IRQ slots;
- handler + context registration;
- duplicate-registration rejection;
- unregister support;
- dispatch by IRQ number;
- handled/spurious/dispatch counters;
- last-IRQ tracking;
- boot-time self-test;
- diagnostic-record export.

## Deliberate safety boundary

The ARM IRQ vector is **still fatal**.

The generic dispatcher is not called directly from the exception vector yet because noOS does not currently own the RM-915 interrupt controller. A correct hardware IRQ path still needs to:

1. initialize/validate QGIC distributor state;
2. initialize the CPU interface;
3. unmask a known timer IRQ only;
4. read the interrupt acknowledge register;
5. pass the decoded IRQ ID to `irq_dispatch()`;
6. write end-of-interrupt;
7. only then return from the ARM IRQ exception.

Until that is validated on physical hardware, an unexpected IRQ remains a crash diagnostic rather than risking an interrupt storm or incorrect EOI sequence.
