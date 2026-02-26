#!/usr/bin/env python3
"""
Safely copy one split unit from a backup splits file into the active splits file.

Recommended first pass:
  - Only import .text and .rodata (and .init if needed).
  - Add data/bss/sdata/sbss/data5 later.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple


LINE_RE = re.compile(
    r"^\s*(\.[A-Za-z0-9_]+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)\s*$"
)


def normalize_section(section: str) -> str:
    if section in (".ctors", ".dtors"):
        return ".rodata"
    if section == ".sdata2":
        return ".data5"
    return section


def parse_units(lines: List[str]) -> Dict[str, List[str]]:
    units: Dict[str, List[str]] = {}
    i = 0
    while i < len(lines):
        line = lines[i]
        if line and not line.startswith("\t") and line.endswith(":") and line != "Sections:":
            name = line[:-1]
            block: List[str] = [line]
            i += 1
            while i < len(lines) and lines[i].startswith("\t"):
                block.append(lines[i])
                i += 1
            units[name] = block
            continue
        i += 1
    return units


def split_header_and_body(lines: List[str]) -> Tuple[List[str], List[str]]:
    if not lines or lines[0] != "Sections:":
        raise ValueError("splits.txt must start with 'Sections:'")
    i = 1
    while i < len(lines) and lines[i].startswith("\t"):
        i += 1
    while i < len(lines) and lines[i].strip() == "":
        i += 1
    return lines[:i], lines[i:]


def build_filtered_block(unit_name: str, block: List[str], allowed_sections: set[str]) -> List[str]:
    out = [f"{unit_name}:"]
    seen = set()
    for raw in block[1:]:
        m = LINE_RE.match(raw.strip())
        if not m:
            continue
        sec = normalize_section(m.group(1))
        start = int(m.group(2), 16)
        end = int(m.group(3), 16)
        if sec not in allowed_sections:
            continue
        if start >= end:
            continue
        key = (sec, start, end)
        if key in seen:
            continue
        seen.add(key)
        out.append(f"\t{sec:<11} start:0x{start:08X} end:0x{end:08X}")
    return out


def remove_unit(lines: List[str], unit_name: str) -> List[str]:
    out: List[str] = []
    i = 0
    target = f"{unit_name}:"
    while i < len(lines):
        if lines[i] == target:
            i += 1
            while i < len(lines) and lines[i].startswith("\t"):
                i += 1
            while i < len(lines) and lines[i].strip() == "":
                i += 1
            continue
        out.append(lines[i])
        i += 1
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--unit", required=True, help="Unit name without trailing ':', e.g. ai.c")
    parser.add_argument(
        "--splits",
        type=Path,
        default=Path("config/GCBE7D/splits.txt"),
        help="Path to active splits file",
    )
    parser.add_argument(
        "--backup",
        type=Path,
        default=Path("config/GCBE7D/splits.elf_import_backup.txt"),
        help="Path to backup/reference splits file",
    )
    parser.add_argument(
        "--sections",
        default=".text,.rodata",
        help="Comma-separated sections to import (default: .text,.rodata)",
    )
    parser.add_argument(
        "--replace",
        action="store_true",
        help="Replace existing unit in active splits if present",
    )
    args = parser.parse_args()

    allowed_sections = {normalize_section(s.strip()) for s in args.sections.split(",") if s.strip()}
    if not allowed_sections:
        print("No sections selected.", file=sys.stderr)
        return 2

    if not args.splits.exists():
        print(f"Missing active splits: {args.splits}", file=sys.stderr)
        return 2
    if not args.backup.exists():
        print(f"Missing backup splits: {args.backup}", file=sys.stderr)
        return 2

    active_lines = args.splits.read_text(encoding="utf-8").splitlines()
    backup_lines = args.backup.read_text(encoding="utf-8").splitlines()
    backup_units = parse_units(backup_lines)

    if args.unit not in backup_units:
        print(f"Unit '{args.unit}' not found in backup.", file=sys.stderr)
        return 1

    new_block = build_filtered_block(args.unit, backup_units[args.unit], allowed_sections)
    if len(new_block) == 1:
        print(f"Unit '{args.unit}' has no usable ranges for sections: {sorted(allowed_sections)}", file=sys.stderr)
        return 1

    active_units = parse_units(active_lines)
    if args.unit in active_units and not args.replace:
        print(f"Unit '{args.unit}' already exists. Use --replace to overwrite.", file=sys.stderr)
        return 1

    # Preserve existing file order as much as possible.
    header, body = split_header_and_body(active_lines)
    body_lines = body
    if args.unit in active_units:
        body_lines = remove_unit(body_lines, args.unit)

    # Ensure body ends with a blank line before appending.
    while body_lines and body_lines[-1].strip() == "":
        body_lines.pop()
    if body_lines:
        body_lines.append("")
    body_lines.extend(new_block)
    body_lines.append("")

    out_lines = header + body_lines
    args.splits.write_text("\n".join(out_lines), encoding="utf-8")
    print(
        f"Imported {args.unit} with sections {', '.join(sorted(allowed_sections))} into {args.splits.as_posix()}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

