#!/usr/bin/env python3
"""
Generate initial .text split units from map-style comments in source files.

This is intentionally conservative:
- only uses lines of the form:
    8004f584 0000bc 8004f584  4 InitTexAnimScripts  Global
- only emits .text ranges
- maps header comments to a sibling .c file with the same basename
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

MAP_LINE_RE = re.compile(
    r"^\s*([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{6,8})\s+"
    r"[0-9A-Fa-f]{8}\s+\d+\s+([A-Za-z_][A-Za-z0-9_.$@?]*)\s+Global\b"
)

SECTION_HEADER = """Sections:
\t.init       type:code align:4
\t.text       type:code align:4
\t.rodata     type:rodata align:4
\t.data       type:data align:4
\t.bss        type:bss align:4
\t.sdata      type:data align:8
\t.sbss       type:bss align:4
\t.data5      type:data align:16
"""


def parse_map_lines(path: Path) -> list[tuple[int, int]]:
    out: list[tuple[int, int]] = []
    try:
        lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
    except OSError:
        return out

    for line in lines:
        m = MAP_LINE_RE.match(line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        size = int(m.group(2), 16)
        out.append((addr, size))
    return out


def target_source(path: Path) -> Path | None:
    if path.suffix.lower() == ".c":
        return path
    if path.suffix.lower() == ".h":
        c_path = path.with_suffix(".c")
        if c_path.exists():
            return c_path
    return None


def build_units(src_root: Path) -> list[tuple[str, int, int, int]]:
    ranges_by_file: dict[Path, list[tuple[int, int]]] = {}
    all_candidates = list(src_root.rglob("*.h")) + list(src_root.rglob("*.c"))
    for candidate in all_candidates:
        funcs = parse_map_lines(candidate)
        if not funcs:
            continue
        dst = target_source(candidate)
        if dst is None:
            continue
        ranges_by_file.setdefault(dst, []).extend(funcs)

    units: list[tuple[str, int, int, int]] = []
    for c_file, funcs in ranges_by_file.items():
        start = min(addr for addr, _ in funcs)
        end = max(addr + size for addr, size in funcs)
        rel = c_file.relative_to(src_root).as_posix()
        units.append((rel, start, end, len(funcs)))
    units.sort(key=lambda item: (item[1], item[0]))
    return units


def write_splits(out_path: Path, units: list[tuple[str, int, int, int]]) -> None:
    lines = [SECTION_HEADER.rstrip(), ""]
    for rel, start, end, _count in units:
        lines.append(f"{rel}:")
        lines.append(f"\t.text       start:0x{start:08X} end:0x{end:08X}")
        lines.append("")
    out_path.write_text("\n".join(lines).rstrip() + "\n", encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate seed splits.txt from map comments")
    parser.add_argument(
        "--root",
        type=Path,
        default=Path("."),
        help="project root (default: current directory)",
    )
    parser.add_argument(
        "--version",
        default="GCBE7D",
        help="config version folder under config/ (default: GCBE7D)",
    )
    args = parser.parse_args()

    root = args.root.resolve()
    src_root = root / "src"
    out_path = root / "config" / args.version / "splits.txt"

    units = build_units(src_root)
    write_splits(out_path, units)

    total_funcs = sum(count for *_rest, count in units)
    print(f"Wrote {out_path} with {len(units)} file units from {total_funcs} map entries")


if __name__ == "__main__":
    main()
