# RM-915 hardware source provenance

noOS should not gain hardware constants from memory, random forum posts, or a nearby Qualcomm SoC when target-specific source is available.

## Lumia 520 bootloader source

Current UART bring-up values were traced to:

- repository: `Android4Lumia/bootloader_msm8227`
- branch: `L520_stable`
- observed source commit in GitHub results: `18eff179c9da3f01fe60a562ca36de5f9ddd0880`

Relevant files:

| File | What noOS derives from it |
| --- | --- |
| `target/msm8960/init.c` | MSM8227 uses GSBI5, GSBI base `0x16400000`, UARTDM base `0x16440000` |
| `platform/msm8960/gpio.c` | GSBI5 UART RX GPIO23 / TX GPIO22, function 1, no pull, 8 mA |
| `platform/msm8960/include/platform/iomap.h` | TLMM base `0x00800000`, clock controller base `0x00900000`, GPIO register formula |
| `platform/msm8960/include/platform/clock.h` | GSBI clock-register formulas, M/N encoding, PLL8 mux id |
| `platform/msm8960/clock.c` | GSBI5 UART clock object and 1.8432 MHz table entry `(pll8, d=1, m=3, n=625)` |
| `platform/msm8960/acpuclock.c` | UART clock requested at 1,843,200 Hz |
| `platform/msm8960/platform.c` | DGT divider value 3 and 6.75 MHz tick rate |
| `platform/msm8960/include/platform/iomap.h` | GIC + GPT/DGT timer addresses |
| `platform/msm_shared/include/gsbi.h` | GSBI protocol shift and I2C/UART protocol code |
| `platform/msm_shared/include/uart_dm.h` | legacy UARTDM register layout and status bits |
| `platform/msm_shared/uart_dm.c` | UARTDM initialization and TX sequencing |

## Values currently encoded in noOS

Target selection:

```text
GSBI id      = 5
GSBI base    = 0x16400000
UARTDM base  = 0x16440000
RX GPIO      = 23
TX GPIO      = 22
GPIO func    = 1
```

Clock/TLMM plan:

```text
TLMM TX cfg  = 0x00801160
TLMM RX cfg  = 0x00801170
TLMM value   = 0x000000C4
HCLK ctl     = 0x00902A40
UART MD reg  = 0x00902A50
UART NS reg  = 0x00902A54
GSBI reset   = 0x00902A5C
MD value     = 0x0003FD8E
NS value     = 0xFD910043
GSBI proto   = 0x00000060
```

These values live in the validation-only full-init plan. The default kernel does not execute that plan.

## Policy for new hardware constants

Before a new physical address or magic register value is allowed into an executing driver:

1. identify a Fame/Lumia-specific source when possible;
2. record the source file and target path here or in the subsystem documentation;
3. isolate writes behind an explicit bring-up feature until tested;
4. add a compile-time/runtime validator or self-test when the value can be derived;
5. keep the default image on the least-invasive path until hardware testing confirms the change.
