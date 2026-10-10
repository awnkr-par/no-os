#!/usr/bin/env python3
from __future__ import annotations

import argparse
import struct
from pathlib import Path

MAGIC = b"ANDROID!"
EXPECTED_PAGE_SIZE = 4096
EXPECTED_KERNEL_ADDR = 0x80208000
EXPECTED_TAGS_ADDR = 0x80200100

HEADER_FORMAT = "<8s10I16s512s32s1024s"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("image", type=Path)
    args = ap.parse_args()

    data = args.image.read_bytes()
    if len(data) < HEADER_SIZE:
        raise SystemExit("boot image too small")

    fields = struct.unpack_from(HEADER_FORMAT, data, 0)
    magic = fields[0]
    kernel_size = fields[1]
    kernel_addr = fields[2]
    ramdisk_size = fields[3]
    tags_addr = fields[7]
    page_size = fields[8]

    problems: list[str] = []

    if magic != MAGIC:
        problems.append("bad Android boot magic")
    if page_size != EXPECTED_PAGE_SIZE:
        problems.append(f"page size {page_size} != {EXPECTED_PAGE_SIZE}")
    if kernel_addr != EXPECTED_KERNEL_ADDR:
        problems.append(f"kernel address 0x{kernel_addr:08x} != 0x{EXPECTED_KERNEL_ADDR:08x}")
    if tags_addr != EXPECTED_TAGS_ADDR:
        problems.append(f"tags address 0x{tags_addr:08x} != 0x{EXPECTED_TAGS_ADDR:08x}")
    if kernel_size == 0:
        problems.append("kernel payload is empty")
    if ramdisk_size != 0:
        problems.append("unexpected ramdisk payload")

    kernel_offset = page_size
    kernel_end = kernel_offset + kernel_size
    if kernel_end > len(data):
        problems.append("kernel payload extends beyond image")

    if problems:
        for problem in problems:
            print(f"error: {problem}")
        raise SystemExit(1)

    print("legacy boot image verified")
    print(f"kernel size: {kernel_size}")
    print(f"kernel addr: 0x{kernel_addr:08x}")
    print(f"tags addr:   0x{tags_addr:08x}")
    print(f"page size:   {page_size}")


if __name__ == "__main__":
    main()
