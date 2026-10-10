# RM-915 early display path

noOS does not initialize the Qualcomm display controller yet.

Instead, it can reuse a framebuffer already configured by the bootloader.

## Supported handoff

noOS supports two bootloader framebuffer descriptions.

### DTB simple-framebuffer

The DTB parser recognizes a `simple-framebuffer` node and reads:

- `reg`
- `width`
- `height`
- `stride`
- `format`

Supported formats:

- `x8r8g8b8`
- `a8r8g8b8`
- `r5g6b5`

### Legacy ATAG_VIDEOLFB

The legacy ATAGS parser also recognizes `ATAG_VIDEOLFB` (`0x54410008`). It consumes the framebuffer base/size, width, height, line length, depth, and RGB bitfield positions. noOS accepts standard RGB565 and 32-bit RGB layouts that match the renderer.

The framebuffer memory range is permanently reserved in the PMM so normal page allocation cannot overwrite the display.

## lk2nd

lk2nd can add a simplefb node to the DTB with an OS command-line option such as:

```text
lk2nd.pass-simplefb=xrgb8888
```

For command-mode panels, `autorefresh` may also be needed depending on actual Fame behavior:

```text
lk2nd.pass-simplefb=xrgb8888,autorefresh
```

## Proof-of-life splash

If a usable simple framebuffer is present, noOS draws a high-contrast splash showing:

```text
NOOS
RM-915
SELFTEST PASS
```

A panic after framebuffer initialization redraws the status as `SELFTEST FAIL`.

This is intentionally primitive. It exists to prove that the kernel reached C, parsed boot data, initialized memory, and completed self-tests without needing UART first.
