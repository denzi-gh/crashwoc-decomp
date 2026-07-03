#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path

from dol import DolImage, decode_branch_target, encode_bl


def parse_int(value: int | str) -> int:
    if isinstance(value, int):
        return value
    return int(value, 0)


def load_symbols(nm: Path, elf: Path) -> dict[str, int]:
    proc = subprocess.run(
        [str(nm), "-n", str(elf)],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    )
    symbols: dict[str, int] = {}
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 3:
            try:
                symbols[parts[2]] = int(parts[0], 16)
            except ValueError:
                pass
    return symbols


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dol", required=True, type=Path)
    parser.add_argument("--elf", required=True, type=Path)
    parser.add_argument("--hooks", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--nm", required=True, type=Path)
    args = parser.parse_args()

    manifest = json.loads(args.hooks.read_text(encoding="utf-8"))
    symbols = load_symbols(args.nm, args.elf)
    dol = DolImage.read(args.dol)
    report = {"input": str(args.dol), "output": str(args.out), "word_patches": [], "hooks": []}

    for patch in manifest.get("word_patches", []):
        address = parse_int(patch["address"])
        expected_opcode = parse_int(patch["original_opcode"])
        patched_opcode = parse_int(patch["patched_opcode"])
        actual_opcode = dol.read_u32(address)
        if actual_opcode != expected_opcode:
            raise SystemExit(
                f"unexpected opcode at 0x{address:08X}: "
                f"got 0x{actual_opcode:08X}, expected 0x{expected_opcode:08X}"
            )
        dol.write_u32(address, patched_opcode)
        report["word_patches"].append(
            {
                "name": patch["name"],
                "address": f"0x{address:08X}",
                "original_opcode": f"0x{actual_opcode:08X}",
                "patched_opcode": f"0x{patched_opcode:08X}",
            }
        )

    for hook in manifest["hooks"]:
        address = parse_int(hook["address"])
        expected_opcode = parse_int(hook["original_opcode"])
        expected_target = parse_int(hook["original_target"])
        wrapper = hook["wrapper"]
        if wrapper not in symbols:
            raise SystemExit(f"wrapper symbol not found in ELF: {wrapper}")
        actual_opcode = dol.read_u32(address)
        if actual_opcode != expected_opcode:
            raise SystemExit(
                f"unexpected opcode at 0x{address:08X}: "
                f"got 0x{actual_opcode:08X}, expected 0x{expected_opcode:08X}"
            )
        actual_target = decode_branch_target(address, actual_opcode)
        if actual_target != expected_target:
            raise SystemExit(
                f"unexpected branch target at 0x{address:08X}: "
                f"got 0x{actual_target:08X}, expected 0x{expected_target:08X}"
            )
        wrapper_target = symbols[wrapper]
        patched_opcode = encode_bl(address, wrapper_target)
        dol.write_u32(address, patched_opcode)
        report["hooks"].append(
            {
                "name": hook["name"],
                "address": f"0x{address:08X}",
                "original_opcode": f"0x{actual_opcode:08X}",
                "original_target": f"0x{actual_target:08X}",
                "wrapper": wrapper,
                "wrapper_target": f"0x{wrapper_target:08X}",
                "patched_opcode": f"0x{patched_opcode:08X}",
            }
        )

    dol.write(args.out)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
