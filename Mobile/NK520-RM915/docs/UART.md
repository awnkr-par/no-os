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
