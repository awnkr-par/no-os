#!/usr/bin/env python3
from __future__ import annotations

import argparse
import struct
from pathlib import Path

MAGIC = 0x4E444941
VERSION = 5
UINT_COUNT = 50
PANIC_BYTES = 64
LOG_BYTES = 256
STRUCT_FORMAT = "<" + ("I" * UINT_COUNT) + f"{PANIC_BYTES}s{LOG_BYTES}s"
STRUCT_SIZE = struct.calcsize(STRUCT_FORMAT)

FIELDS = [
    "magic", "version", "size", "checksum",
    "stage", "detail",
    "boot_protocol", "boot_data_address", "boot_data_size",
    "total_memory_bytes", "reserved_range_count",
    "boot_option_flags", "boot_log_level",
    "cpu_midr", "cpu_cpsr", "cpu_sctlr",
    "pmm_total_pages", "pmm_free_pages", "pmm_permanent_pages",
    "pmm_allocated_pages",
    "mmu_table_address", "mmu_mapped_sections", "mmu_first_section",
    "mmu_last_section", "mmu_validated",
    "uart_base", "uart_bytes_sent", "uart_tx_timeouts", "uart_flags",
    "framebuffer_address", "framebuffer_width", "framebuffer_height",
    "framebuffer_stride", "framebuffer_format",
    "selftests_passed", "selftests_failed",
    "klog_bytes_written", "klog_bytes_overwritten", "klog_wraps", "klog_length",
    "scheduler_ticks", "scheduler_tasks", "scheduler_ready",
    "scheduler_sleeping", "scheduler_switches_planned",
    "irq_registered", "irq_dispatches", "irq_handled", "irq_spurious", "irq_last",
]

STAGES = {
    1: "reset",
    2: "c-entry",
    3: "boot-info",
    4: "memory",
    5: "heap",
    6: "selftest",
    7: "mmu-plan",
    8: "ready",
    0xFF: "panic",
}

PROTOCOLS = {
    0: "unknown",
    1: "ATAGS",
    2: "DTB",
}

PIXEL_FORMATS = {
    0: "unknown",
    1: "XRGB8888",
    2: "ARGB8888",
    3: "RGB565",
}


def fnv1a_without_checksum(data: bytes) -> int:
    h = 2166136261
    for i, byte in enumerate(data[:STRUCT_SIZE]):
        if 12 <= i < 16:
            continue
        h ^= byte
        h = (h * 16777619) & 0xFFFFFFFF
    return h


def c_string(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("utf-8", "replace")


def decode(data: bytes, offset: int) -> dict[str, int | str]:
    if offset < 0 or offset + STRUCT_SIZE > len(data):
        raise ValueError(
            f"need {STRUCT_SIZE} bytes at offset 0x{offset:x}, "
            f"input is only {len(data)} bytes"
        )

    chunk = data[offset:offset + STRUCT_SIZE]
    unpacked = struct.unpack(STRUCT_FORMAT, chunk)

    values = dict(zip(FIELDS, unpacked[:UINT_COUNT]))
    values["panic_reason"] = c_string(unpacked[UINT_COUNT])
    values["log_tail"] = c_string(unpacked[UINT_COUNT + 1])
    values["computed_checksum"] = fnv1a_without_checksum(chunk)
    return values


def print_record(v: dict[str, int | str]) -> None:
    magic = int(v["magic"])
    checksum = int(v["checksum"])
    computed = int(v["computed_checksum"])

    print(f"magic:          0x{magic:08X} " + ("OK" if magic == MAGIC else "BAD"))
    print(f"version:        {v['version']}")
    print(f"record size:    {v['size']} bytes")
    print(
        f"checksum:       0x{checksum:08X} "
        + ("OK" if checksum == computed else f"BAD expected 0x{computed:08X}")
    )
    print(f"stage:          {STAGES.get(int(v['stage']), 'unknown')} ({v['stage']})")
    print(f"detail:         0x{int(v['detail']):08X}")
    print(
        f"boot protocol:  {PROTOCOLS.get(int(v['boot_protocol']), 'unknown')} "
        f"({v['boot_protocol']})"
    )
    print(f"boot data:      0x{int(v['boot_data_address']):08X} + 0x{int(v['boot_data_size']):X}")
    print(f"RAM bytes:      0x{int(v['total_memory_bytes']):08X}")
    print(f"reserved ranges:{v['reserved_range_count']}")
    flags = int(v["boot_option_flags"])
    print(
        "boot options:   "
        f"splash={bool(flags & 1)} selftest={bool(flags & 2)} "
        f"mmu-plan={bool(flags & 4)} sched-test={bool(flags & 8)} "
        f"loglevel={v['boot_log_level']}"
    )
    print(f"CPU MIDR:       0x{int(v['cpu_midr']):08X}")
    print(f"CPU CPSR:       0x{int(v['cpu_cpsr']):08X}")
    print(f"CPU SCTLR:      0x{int(v['cpu_sctlr']):08X}")
    print(
        "PMM pages:      "
        f"total={v['pmm_total_pages']} free={v['pmm_free_pages']} "
        f"permanent={v['pmm_permanent_pages']} allocated={v['pmm_allocated_pages']}"
    )
    print(
        "MMU plan:       "
        f"table=0x{int(v['mmu_table_address']):08X} "
        f"sections={v['mmu_mapped_sections']} "
        f"range={v['mmu_first_section']}..{v['mmu_last_section']} "
        f"validated={bool(v['mmu_validated'])}"
    )
    print(
        "UART:           "
        f"base=0x{int(v['uart_base']):08X} "
        f"bytes={v['uart_bytes_sent']} timeouts={v['uart_tx_timeouts']} "
        f"adopted={bool(int(v['uart_flags']) & 1)} "
        f"healthy={bool(int(v['uart_flags']) & 2)}"
    )
    print(
        "framebuffer:    "
        f"0x{int(v['framebuffer_address']):08X} "
        f"{v['framebuffer_width']}x{v['framebuffer_height']} "
        f"stride={v['framebuffer_stride']} "
        f"{PIXEL_FORMATS.get(int(v['framebuffer_format']), 'unknown')}"
    )
    print(
        f"selftests:      pass={v['selftests_passed']} "
        f"fail={v['selftests_failed']}"
    )
    print(
        "klog:           "
        f"written={v['klog_bytes_written']} "
        f"overwritten={v['klog_bytes_overwritten']} "
        f"wraps={v['klog_wraps']} retained={v['klog_length']}"
    )
    print(
        "scheduler:      "
        f"ticks={v['scheduler_ticks']} tasks={v['scheduler_tasks']} "
        f"ready={v['scheduler_ready']} sleeping={v['scheduler_sleeping']} "
        f"planned={v['scheduler_switches_planned']}"
    )
    print(
        "IRQ core:       "
        f"registered={v['irq_registered']} dispatches={v['irq_dispatches']} "
        f"handled={v['irq_handled']} spurious={v['irq_spurious']} "
        f"last={v['irq_last']}"
    )
    print(f"panic reason:   {v['panic_reason'] or '(none)'}")
    print("--- log tail ---")
    print(v["log_tail"] or "(empty)")


def make_selftest_record() -> bytes:
    ints = [0] * UINT_COUNT
    ints[0] = MAGIC
    ints[1] = VERSION
    ints[2] = STRUCT_SIZE
    ints[4] = 8
    ints[6] = 2
    ints[9] = 0x20000000
    ints[11] = 0xF
    ints[12] = 4
    ints[16] = 131072
    ints[17] = 120000
    ints[20] = 0x80210000
    ints[21] = 512
    ints[24] = 1
    ints[34] = 12
    ints[35] = 0
    ints[36] = 9000
    ints[37] = 808
    ints[38] = 2
    ints[39] = 8192
    ints[40] = 2
    ints[41] = 2
    ints[42] = 1
    ints[43] = 1
    ints[44] = 4
    ints[45] = 1
    ints[46] = 2
    ints[47] = 1
    ints[48] = 1
    ints[49] = 43

    panic = b"".ljust(PANIC_BYTES, b"\0")
    log = b"decoder v4 selftest".ljust(LOG_BYTES, b"\0")
    raw = bytearray(struct.pack(STRUCT_FORMAT, *ints, panic, log))
    checksum = fnv1a_without_checksum(raw)
    struct.pack_into("<I", raw, 12, checksum)
    return bytes(raw)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("dump", nargs="?", type=Path)
    ap.add_argument("--offset", type=lambda x: int(x, 0), default=0)
    ap.add_argument("--selftest", action="store_true")
    args = ap.parse_args()

    if args.selftest:
        data = make_selftest_record()
        values = decode(data, 0)
        assert values["magic"] == MAGIC
        assert values["checksum"] == values["computed_checksum"]
        assert values["stage"] == 8
        assert values["boot_protocol"] == 2
        assert values["selftests_passed"] == 12
        assert values["boot_option_flags"] == 0xF
        assert values["klog_wraps"] == 2
        assert values["scheduler_tasks"] == 2
        assert values["irq_handled"] == 1
        assert values["irq_last"] == 43
        print("diagnostic decoder v5 selftest: OK")
        return

    if args.dump is None:
        ap.error("dump file is required unless --selftest is used")

    values = decode(args.dump.read_bytes(), args.offset)
    print_record(values)


if __name__ == "__main__":
    main()
