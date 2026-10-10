# noOS RM-915 boot options

noOS parses its own options from the kernel command line after the bootloader handoff.

Current options:

| Option | Default | Meaning |
| --- | --- | --- |
| `noos.splash` | `1` | Draw boot/runtime framebuffer UI when a usable framebuffer exists |
| `noos.selftest` | `1` | Run kernel bring-up self-tests |
| `noos.mmu-plan` | `1` | Build and validate the ARMv7 identity-map plan without enabling it |
| `noos.sched-test` | `1` | Exercise scheduler state-machine tests |
| `noos.loglevel` | `2` | Numeric logging policy value reserved for later filtering |

Boolean values accept `1/0`, `on/off`, `true/false`, and `yes/no`.

Numeric values accept decimal or `0x` hexadecimal.

Example:

```text
noos.splash=1 noos.selftest=1 noos.mmu-plan=0 noos.loglevel=4
```

The parser is deliberately allocation-free and does not depend on compiler integer-division helpers.


## Build-time composition

The Makefile keeps bootloader options and noOS options separate:

- `LK2ND_CMDLINE` — lk2nd-specific flags such as simplefb/autorefresh.
- `NOOS_CMDLINE` — noOS runtime options.
- `BOOT_CMDLINE` — automatically composed from both.

This makes hardware experiments less error-prone because changing an lk2nd framebuffer flag does not accidentally remove noOS safety/test flags.
