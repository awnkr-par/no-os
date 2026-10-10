#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
from pathlib import Path


SYMBOLS = [
    "_start",
    "__image_start",
    "__image_end",
    "__diag_start",
    "__diag_end",
    "__mmu_l1_start",
    "__mmu_l1_end",
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def symbol_map(nm: str, elf: Path) -> dict[str, int]:
    proc = subprocess.run(
        [nm, "-n", str(elf)],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    wanted = set(SYMBOLS)
    result: dict[str, int] = {}

    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) < 3:
            continue
        address, _, name = parts[0], parts[1], parts[2]
        if name in wanted:
            result[name] = int(address, 16)

    missing = sorted(wanted - result.keys())
    if missing:
        raise SystemExit(f"missing required symbols: {', '.join(missing)}")

    return result


def file_info(path: Path) -> dict[str, int | str]:
    return {
        "path": path.name,
        "size": path.stat().st_size,
        "sha256": sha256(path),
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--elf", type=Path, required=True)
    ap.add_argument("--bin", dest="binary", type=Path, required=True)
    ap.add_argument("--boot", type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--nm", default="arm-none-eabi-nm")
    ap.add_argument("--boot-cmdline", default="")
    args = ap.parse_args()

    symbols = symbol_map(args.nm, args.elf)

    manifest = {
        "format": "noos-rm915-build-manifest",
        "version": 1,
        "target": "nokia-rm915",
        "architecture": "armv7-a",
        "entrypoint": f"0x{symbols['_start']:08x}",
        "image": {
            "start": f"0x{symbols['__image_start']:08x}",
            "end": f"0x{symbols['__image_end']:08x}",
            "size": symbols["__image_end"] - symbols["__image_start"],
        },
        "diagnostic": {
            "start": f"0x{symbols['__diag_start']:08x}",
            "end": f"0x{symbols['__diag_end']:08x}",
            "size": symbols["__diag_end"] - symbols["__diag_start"],
        },
        "mmu_l1": {
            "start": f"0x{symbols['__mmu_l1_start']:08x}",
            "end": f"0x{symbols['__mmu_l1_end']:08x}",
            "size": symbols["__mmu_l1_end"] - symbols["__mmu_l1_start"],
            "alignment": 16384,
            "enabled": False,
        },
        "boot_cmdline": args.boot_cmdline,
        "artifacts": {
            "elf": file_info(args.elf),
            "bin": file_info(args.binary),
            "boot_img": file_info(args.boot),
        },
    }

    if symbols["_start"] != 0x80208000:
        raise SystemExit(
            f"unexpected entrypoint 0x{symbols['_start']:08x}; expected 0x80208000"
        )

    if symbols["__mmu_l1_start"] & 0x3FFF:
        raise SystemExit("MMU L1 table is not 16 KiB aligned")

    if manifest["mmu_l1"]["size"] != 16384:
        raise SystemExit(
            f"MMU L1 table size is {manifest['mmu_l1']['size']}, expected 16384"
        )

    if manifest["diagnostic"]["size"] <= 0:
        raise SystemExit("diagnostic section is empty")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()
