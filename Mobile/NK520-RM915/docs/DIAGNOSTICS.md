# noOS RM-915 diagnostic record

noOS keeps a fixed-format diagnostic structure in the dedicated ELF section:

```text
.noos_diag
```

The linker exports:

```text
__diag_start
__diag_end
```

The structure is outside `.bss`, explicitly initialized by C, and included inside the kernel image's reserved physical range.

## Captured fields

Version 2 records:

- boot stage and detail value;
- boot protocol;
- boot-data address and full size;
- RAM total and number of reserved ranges;
- MIDR, CPSR, and SCTLR;
- PMM total/free/permanent/allocated page counts;
- MMU L1 table address and validated mapping range;
- framebuffer location/geometry/format;
- self-test pass/fail counts;
- panic reason;
- tail of the in-memory early console;
- FNV-1a checksum over the record excluding the checksum field.

## Decode a RAM dump

If a debugger or recovery tool gives you bytes beginning exactly at `__diag_start`:

```sh
python3 tools/decode_diag.py diag.bin
```

For a larger memory dump, use the offset from the beginning of that dump:

```sh
python3 tools/decode_diag.py memory.bin --offset 0x12340
```

The exact runtime address of `__diag_start` is available from:

```sh
arm-none-eabi-nm build/noos.elf | grep __diag_start
```

The decoder has a host-only sanity test:

```sh
python3 tools/decode_diag.py --selftest
```

CI runs this self-test so the Python layout stays synchronized with the C record format.
