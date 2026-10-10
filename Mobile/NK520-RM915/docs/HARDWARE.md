# RM-915 hardware bring-up notes

## Verified platform facts

- Device family: Nokia Lumia 520, codename **Fame**.
- RM-915 is the `fame_lta` variant.
- SoC: Qualcomm MSM8227 (ARMv7 / dual-core Krait family).
- RAM: 512 MiB on shipping devices.
- eMMC: 8 GiB.
- PMIC: Qualcomm PM8038.
- Display: 480 × 800.

## Verified legacy boot-image layout

The historical postmarketOS Fame device configuration uses:

| Item | Value |
| --- | ---: |
| Page size | `0x1000` (4096) |
| Base | `0x80200000` |
| Kernel offset | `0x00008000` |
| Kernel load address | `0x80208000` |
| RAM-disk offset | `0x02000000` |
| Second-stage offset | `0x00f00000` |
| Tags offset | `0x00000100` |
| ATAGS address | `0x80200100` |

The downstream Linux command line uses `console=ttyHSL0,115200,n8`, establishing ttyHSL0 as the intended debug console for that software stack.

## ARM32 boot handoff

Recent U-Boot work tested on a Nokia Lumia 520 confirms that the phone can be chained directly from SBL3. In that path, U-Boot receives legacy ARM ATAGS. ARM32 passes the ATAGS/FDT pointer in `r2`.

noOS preserves `r0`, `r1`, and `r2` at entry and detects whether `r2` contains legacy ATAGS or a flattened device tree. The DTB path can also consume a bootloader-provided `simple-framebuffer`, allowing first visual output without programming the Qualcomm display controller.

## UART status

The service schematic exposes `DBG_UART_TX` and `DBG_UART_RX` signals on the MSM8227, but the exact RM-915 UART controller base, clock source, GSBI mode, and pinmux sequence are not yet sufficiently verified for noOS to write MMIO safely.

**Do not copy a UART base from a random MSM8960/MSM8930 device.** Related Qualcomm SoCs share IP blocks but board routing and GSBI selection vary.

The next UART milestone is to identify the exact ttyHSL0 platform resource from the Android4Lumia Fame kernel/board files or from a known-good boot log generated on Fame hardware, then port the minimal UARTDM initialization sequence.

## Flashing warning

`build/noos-boot.img` is structurally packaged for the known legacy Fame Android boot layout, but that does **not** make flashing it risk-free. Keep a known-good FFU/recovery path available and prefer temporary/chainloaded boot methods while bring-up is incomplete.
