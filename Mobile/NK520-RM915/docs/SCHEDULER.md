# Scheduler core

noOS now contains the state-management half of a scheduler, but **does not perform CPU context switches yet**.

That distinction is intentional.

## Task states

- unused
- ready
- running
- sleeping
- blocked
- dead

Up to 16 task records are currently supported.

## Implemented behavior

- task registration;
- state transitions;
- sleep-until tick deadlines;
- wakeup on scheduler ticks;
- round-robin next-task selection;
- scheduler statistics;
- boot-time state-machine self-test.

## Not implemented yet

- saving/restoring ARM register contexts;
- per-task kernel stacks;
- IRQ-driven preemption;
- userspace mode;
- address-space switching;
- synchronization primitives.

The next scheduler milestone should happen only after the hardware timer and interrupt-controller path can deliver reliable periodic IRQs. Until then the current scheduler is a deterministic planning/state subsystem, not a fake multitasking demo.
