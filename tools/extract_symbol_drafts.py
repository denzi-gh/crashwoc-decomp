#!/usr/bin/env python3
"""
Extract draft symbol files from existing source annotations.

This script produces:
1) A high-confidence symbols draft from map-style function lists in comments.
2) A lower-confidence candidate list from DWARF dump globals.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path
from typing import Dict, Iterable, List, Set, Tuple


MAP_LINE_RE = re.compile(
    r"^\s*([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{6,8})\s+"
    r"[0-9A-Fa-f]{8}\s+\d+\s+([A-Za-z_][A-Za-z0-9_.$@?]*)\s+Global\b"
)

DWARF_ADDR_RE = re.compile(r":\s*0x([0-9A-Fa-f]{8});\s*//\s*size:\s*0x([0-9A-Fa-f]+)")
IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")


def iter_files(root: Path, patterns: Iterable[str]) -> Iterable[Path]:
    for pattern in patterns:
        yield from root.rglob(pattern)


def parse_map_style_functions(src_dir: Path) -> Tuple[List[str], Dict[str, Set[str]], int]:
    out: List[str] = []
    seen: Set[Tuple[str, str, str]] = set()
    conflicts: Dict[str, Set[str]] = {}
    total_matches = 0

    for path in iter_files(src_dir, ("*.h", "*.c", "*.txt")):
        try:
            lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
        except OSError:
            continue
        for line in lines:
            m = MAP_LINE_RE.match(line)
            if not m:
                continue
            total_matches += 1
            addr, size, name = m.group(1).lower(), m.group(2).lower(), m.group(3)
            key = (name, addr, size)
            if key in seen:
                continue
            seen.add(key)
            conflicts.setdefault(name, set()).add(addr)
            out.append(
                f"{name} = .text:0x{addr}; // type:function size:0x{size} scope:global"
            )

    out.sort()
    return out, conflicts, total_matches


def parse_dwarf_candidates(dwarf_path: Path) -> List[str]:
    if not dwarf_path.is_file():
        return []

    out: List[str] = []
    seen: Set[Tuple[str, str]] = set()

    try:
        lines = dwarf_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    except OSError:
        return []

    for line in lines:
        if "Compile unit:" in line:
            continue
        if line.lstrip().startswith("//"):
            continue

        m = DWARF_ADDR_RE.search(line)
        if not m:
            continue
        addr = m.group(1).lower()
        size = m.group(2).lower()

        # Left side before ":" should include the declared symbol name.
        left = line.split(":", 1)[0]
        ids = IDENT_RE.findall(left)
        if not ids:
            continue
        name = ids[-1]
        if name in {"struct", "union", "enum", "typedef", "static", "const"}:
            continue

        key = (name, addr)
        if key in seen:
            continue
        seen.add(key)

        out.append(
            f"# {name} = <section>:0x{addr}; // type:object size:0x{size} "
            f"scope:global (from DWARF, section unknown)"
        )

    out.sort()
    return out


def find_conflicts(conflicts: Dict[str, Set[str]]) -> List[str]:
    out = []
    for name, addrs in sorted(conflicts.items()):
        if len(addrs) > 1:
            addr_list = ", ".join(sorted(addrs))
            out.append(f"# CONFLICT {name}: {addr_list}")
    return out


def main() -> None:
    parser = argparse.ArgumentParser(description="Extract draft symbol files.")
    parser.add_argument(
        "--version",
        default="GCBE7D",
        help="Game version folder under config/ (default: GCBE7D)",
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=Path("."),
        help="Project root directory (default: current directory)",
    )
    args = parser.parse_args()

    root = args.root.resolve()
    src_dir = root / "src"
    out_dir = root / "config" / args.version
    out_dir.mkdir(parents=True, exist_ok=True)

    symbols_out = out_dir / "symbols.autogen.txt"
    dwarf_out = out_dir / "symbols.dwarf_candidates.txt"

    map_symbols, conflicts, total_matches = parse_map_style_functions(src_dir)
    conflict_lines = find_conflicts(conflicts)
    dwarf_candidates = parse_dwarf_candidates(src_dir / "dump_alphaNGCport_DWARF.txt")

    symbols_lines: List[str] = [
        "# Auto-generated draft symbols from map-style header comments.",
        "# Review before merging into symbols.txt.",
        "",
        *map_symbols,
    ]
    if conflict_lines:
        symbols_lines.extend(["", "# Name conflicts detected (same symbol name, different address):"])
        symbols_lines.extend(conflict_lines)

    dwarf_lines: List[str] = [
        "# Auto-generated DWARF candidates.",
        "# These are NOT directly safe to use in symbols.txt because section is unknown.",
        "# Convert only verified entries.",
        "",
        *dwarf_candidates,
    ]

    symbols_out.write_text("\n".join(symbols_lines) + "\n", encoding="utf-8")
    dwarf_out.write_text("\n".join(dwarf_lines) + "\n", encoding="utf-8")

    print(f"Wrote {symbols_out} ({len(map_symbols)} symbols, {total_matches} raw matches)")
    print(f"Wrote {dwarf_out} ({len(dwarf_candidates)} candidates)")
    if conflict_lines:
        print(f"Detected {len(conflict_lines)} symbol name conflicts")


if __name__ == "__main__":
    main()

