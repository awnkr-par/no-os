#!/usr/bin/env python3
"""Create a legacy Android boot image for Nokia Lumia 520 (Fame).

This intentionally implements only the v0 header format needed by the known
postmarketOS/Android Fame layout. It does not flash anything.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path

MAGIC = b"ANDROID!"
PAGE_SIZE = 4096
BASE = 0x80200000
KERNEL_OFFSET = 0x00008000
RAMDISK_OFFSET = 0x02000000
SECOND_OFFSET = 0x00F00000
TAGS_OFFSET = 0x00000100


def pad(data: bytes, page_size: int = PAGE_SIZE) -> bytes:
    remainder = len(data) % page_size
    return data if remainder == 0 else data + b"\0" * (page_size - remainder)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("kernel", type=Path)
    ap.add_argument("output", type=Path)
    ap.add_argument("--name", default="noOS-RM915")
    ap.add_argument("--cmdline", default="")
    args = ap.parse_args()

    kernel = args.kernel.read_bytes()
    ramdisk = b""
    second = b""

    sha = hashlib.sha1()
    sha.update(kernel)
    sha.update(struct.pack("<I", len(kernel)))
    sha.update(ramdisk)
    sha.update(struct.pack("<I", len(ramdisk)))
    sha.update(second)
    sha.update(struct.pack("<I", len(second)))
    digest = sha.digest() + b"\0" * 12

    name = args.name.encode("ascii", "strict")[:15].ljust(16, b"\0")
    cmdline = args.cmdline.encode("ascii", "strict")
    primary = cmdline[:511].ljust(512, b"\0")
    extra = cmdline[511:1534].ljust(1024, b"\0")

    header = struct.pack(
        "<8s10I16s512s32s1024s",
        MAGIC,
        len(kernel), BASE + KERNEL_OFFSET,
        len(ramdisk), BASE + RAMDISK_OFFSET,
        len(second), BASE + SECOND_OFFSET,
        BASE + TAGS_OFFSET,
        PAGE_SIZE,
        0,
        0,
        name,
        primary,
        digest,
        extra,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(pad(header) + pad(kernel))

    print(f"boot image: {args.output}")
    print(f"kernel load: 0x{BASE + KERNEL_OFFSET:08x}")
    print(f"tags addr:   0x{BASE + TAGS_OFFSET:08x}")
    print(f"page size:   {PAGE_SIZE}")


if __name__ == "__main__":
    main()
