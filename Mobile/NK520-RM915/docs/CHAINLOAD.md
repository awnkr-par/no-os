# RM-915 temporary chainload plan

The goal of the first hardware test is **temporary execution**, not permanent installation.

## Intended path

1. Keep a known-good RM-915 recovery/FFU path available.
2. Use an unlocked Lumia boot chain capable of starting lk2nd.
3. Enter lk2nd Fastboot mode.
4. Boot noOS from RAM with a boot image rather than replacing the operating system.
5. Prefer a DTB handoff with lk2nd simple-framebuffer enabled.

## Why DTB now matters

noOS accepts either legacy ARM ATAGS or a flattened device tree (DTB) passed in r2.

DTB is preferred for the first visible boot because lk2nd can describe a bootloader-provided `simple-framebuffer`. noOS parses that node and can draw directly into the existing framebuffer without programming the Qualcomm MDP/display clocks.

The relevant lk2nd OS command-line option is:

```text
lk2nd.pass-simplefb=xrgb8888
```

If the panel requires command-mode refresh support, an lk2nd configuration may also need:

```text
lk2nd.pass-simplefb=xrgb8888,autorefresh
```

Do not assume `autorefresh` is required until the actual Fame/lk2nd behavior is observed.

## Expected first-boot behavior

When a valid `simple-framebuffer` node is supplied, noOS should paint a dark screen containing:

```text
NOOS
RM-915
SELFTEST PASS
```

A panic after framebuffer initialization changes the splash to `SELFTEST FAIL`.

If there is no usable framebuffer handoff, the kernel still records boot protocol, CPU state, memory banks, PMM state, self-test result, last panic/fault, and boot-stage breadcrumbs in RAM.

## Fastboot safety boundary

For early experiments, use a temporary boot command when supported:

```sh
fastboot boot noos-boot.img
```

Do **not** switch to a permanent `fastboot flash boot ...` workflow until the exact Lumia/lk2nd partition-offset behavior is confirmed on the device and recovery has been tested.

## Success criteria

The first phone test is successful if any one of these happens:

1. the noOS splash appears;
2. a debugger/RAM capture shows `NOOS_STATUS_MAGIC` and a progressed boot stage;
3. later UART work exposes the early console.

A black screen alone is not enough to conclude that the kernel failed; the bootloader may simply not have passed a framebuffer usable by noOS.
