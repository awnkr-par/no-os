# RM-915 UART bring-up

This file documents only values traced to the Android4Lumia Lumia 520 stable LK source tree.

## Verified Lumia stable LK path

The L520 stable bootloader uses the MSM8960-family target code for MSM8227.

For MSM8227 / MSM8627 target machine types, `target_uart_init()` selects:

- GSBI instance: **5**
- GSBI base: **0x16400000**
- UARTDM base: **0x16440000**

The matching MSM8960 GPIO code configures GSBI5 UART as:

- RX: GPIO **23**, function **1**
- TX: GPIO **22**, function **1**
- no pull
- 8 mA drive

The shared UARTDM implementation selects 115200 baud and configures the block as 8-N-1.

## Why noOS currently uses handoff mode

LK already initializes this UART before chainloading.

Rather than immediately duplicating the entire Qualcomm clock framework, noOS has an opt-in **handoff/adopt** backend. It assumes LK/lk2nd left GSBI5 UARTDM running and only uses the existing TX path.

This avoids changing:

- GSBI clock roots;
- peripheral clock votes;
- TLMM mux state;
- GSBI protocol mode;
- UART mode/reset state.

The safe/default build does not touch UART MMIO.

## Build a UART handoff image

```sh
make -f GNUmakefile UARTDM_HANDOFF=1
```

CI also builds this mode into a separate `build-uart/` directory.

When enabled, the early console is mirrored live to UARTDM while still being retained in the 4 KiB RAM log.

## TX implementation

The handoff driver uses the legacy GSBI UARTDM register layout at `0x16440000` and bounded polling.

Important offsets used:

| Register | Offset |
| --- | ---: |
| SR | `0x008` |
| CR | `0x010` |
| ISR | `0x014` |
| NO_CHARS_FOR_TX | `0x040` |
| TX FIFO | `0x070` |

Every poll has a finite budget. If UART stops responding, noOS marks the transport unhealthy and continues kernel bring-up instead of hanging forever.

The diagnostic record tracks:

- UART base;
- bytes sent;
- TX timeout count;
- adopted flag;
- healthy flag.

## Full takeover: not enabled yet

The Lumia LK source also confirms the UART clock target is 1.8432 MHz and the UARTDM CSR is configured for 115200 baud.

A complete noOS-owned initialization still needs a careful port of the Qualcomm clock enable/vote sequence. Until that is implemented and validated, noOS does **not** rewrite clocks or TLMM by default.

That separation is intentional: UART handoff is useful for lk2nd testing now, while full standalone UART initialization can be added later without risking the first phone boot.

## Verified full-init plan (not executed)

noOS also carries a validation-only plan derived from the same L520 stable LK sources. It is self-tested but **never written to hardware**.

| Item | Address / value |
| --- | ---: |
| TLMM TX GPIO22 config | `0x00801160` |
| TLMM RX GPIO23 config | `0x00801170` |
| TLMM mux/drive value | `0x000000C4` |
| GSBI5 control | `0x16400000` |
| GSBI I2C/UART protocol value | `0x00000060` |
| GSBI5 HCLK control | `0x00902A40` |
| GSBI5 UART MD | `0x00902A50` |
| GSBI5 UART NS | `0x00902A54` |
| GSBI5 reset | `0x00902A5C` |
| 1.8432 MHz MD value | `0x0003FD8E` |
| 1.8432 MHz NS value | `0xFD910043` |

The M/N values come from the MSM8960 clock table entry:

```text
F_GSBI_UART(1843200, pll8, 1, 3, 625)
```

and the actual `MD16` / `NS` encodings in the same clock implementation.

`uartdm_validate_full_init_plan()` recomputes the addresses and M/N encodings instead of merely comparing against another copy of the constants. The boot self-test fails if the plan drifts.

### Why this is still not the default

Knowing the final register addresses and rate values is not the same as safely reproducing Qualcomm's full clock framework. The clock branch/root enable ordering, PLL8 availability, halt-state polling, reset sequencing, and barriers must all be ported before noOS owns UART from a cold hardware state.

For lk2nd chainloading, adopting LK's already-initialized UART remains the lower-risk debug path.
