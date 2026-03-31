#!/usr/bin/env python3
"""Automated compiler flag sweep for matching optimization.

Tests GCC 2.95.2 (ProDG 3.5) compiler flags one-at-a-time or in combination
to find the set that produces the best match for a given unit/function.

Usage:
    python tools/ai_flag_sweep.py camera.c
    python tools/ai_flag_sweep.py camera.c --function MoveGameCamera
    python tools/ai_flag_sweep.py camera.c --flags="-fno-gcse,-fno-cse-follow-jumps"
    python tools/ai_flag_sweep.py camera.c --combos 2   # Try all 2-flag combos
"""
from __future__ import annotations

import argparse
import itertools
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONFIGURE_PY = ROOT / "configure.py"
DEFAULT_VERSION = "GCBE7D"
DEFAULT_TOOLCHAIN = "prodg35"

# Flags known to affect CSE, register allocation, and scheduling in GCC 2.95.2
DEFAULT_FLAGS = [
    "-fno-gcse",
    "-fno-cse-follow-jumps",
    "-fno-cse-skip-blocks",
    "-fno-expensive-optimizations",
    "-fno-rerun-cse-after-loop",
    "-fno-schedule-insns",
    "-fno-schedule-insns2",
    "-fno-force-mem",
    "-fno-force-addr",
    "-fno-strength-reduce",
    "-fno-peephole",
    "-fno-move-all-movables",
    "-fno-reduce-all-givs",
    "-fno-caller-saves",
    "-fno-inline-functions",
    "-fno-keep-inline-functions",
]


def find_unit_line(source: str, unit_stem: str) -> tuple[int, str] | None:
    """Find the Object(...) line for the given unit in configure.py source."""
    for i, line in enumerate(source.splitlines()):
        # Match: Object(NonMatching, "camera.c" , source="gamecode/camera.c"),
        pattern = rf'Object\(\s*\w+\s*,\s*"{re.escape(unit_stem)}"'
        if re.search(pattern, line):
            return i, line
    return None


def modify_configure(unit_stem: str, extra_cflags: list[str]) -> str:
    """Modify configure.py to add extra_cflags to the given unit. Returns original source."""
    original = CONFIGURE_PY.read_text(encoding="utf-8")
    result = find_unit_line(original, unit_stem)
    if result is None:
        raise ValueError(f"Could not find unit '{unit_stem}' in configure.py")

    lineno, line = result
    lines = original.splitlines(keepends=True)

    # Build the new line
    if "extra_cflags=" in line:
        # Replace existing extra_cflags
        new_line = re.sub(
            r'extra_cflags=\[[^\]]*\]',
            f'extra_cflags={json.dumps(extra_cflags)}',
            line,
        )
    else:
        # Add extra_cflags before the closing paren
        flags_str = f'extra_cflags={json.dumps(extra_cflags)}'
        # Insert before the last closing )
        new_line = re.sub(
            r'\)\s*,\s*$',
            f', {flags_str}),\n',
            line,
        )
        if new_line == line:
            # Fallback: insert before )
            new_line = line.rstrip().rstrip(",").rstrip(")") + f", {flags_str}),"
            if not new_line.endswith("\n"):
                new_line += "\n"

    lines[lineno] = new_line
    CONFIGURE_PY.write_text("".join(lines), encoding="utf-8")
    return original


def restore_configure(original: str) -> None:
    """Restore configure.py to its original content."""
    CONFIGURE_PY.write_text(original, encoding="utf-8")


def run_configure(version: str, toolchain: str) -> bool:
    """Run configure.py. Returns True on success."""
    result = subprocess.run(
        [sys.executable, str(CONFIGURE_PY), "--version", version, "--toolchain", toolchain],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
    )
    return result.returncode == 0


def build_object(object_path: str) -> bool:
    """Build a single object file with ninja. Returns True on success."""
    result = subprocess.run(
        ["ninja", object_path],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
    )
    return result.returncode == 0


def get_match_percent(unit_stem: str, function_name: str | None = None) -> float | None:
    """Read the match percentage from report.json after a build."""
    report_path = ROOT / "build" / DEFAULT_VERSION / "report.json"
    if not report_path.is_file():
        return None

    with open(report_path, "r") as f:
        report = json.load(f)

    for unit in report.get("units", []):
        unit_name = unit.get("name", "")
        if unit_stem.replace(".c", "") not in unit_name:
            continue
        for fn in unit.get("functions", []):
            if function_name and fn.get("name") != function_name:
                continue
            return fn.get("fuzzy_match_percent")
        # If no specific function requested, return unit-level
        if not function_name:
            measures = unit.get("measures", {})
            return measures.get("fuzzy_match_percent")
    return None


def touch_source(source_path: Path) -> None:
    """Touch the source file to force rebuild."""
    if source_path.is_file():
        source_path.touch()


def resolve_source_path(unit_stem: str) -> Path | None:
    """Find the source file for a unit."""
    configure_src = CONFIGURE_PY.read_text(encoding="utf-8")
    m = re.search(rf'"{re.escape(unit_stem)}"[^)]*source="([^"]+)"', configure_src)
    if m:
        return ROOT / "src" / m.group(1)
    # Try direct path
    for candidate in ROOT.rglob(f"src/**/{unit_stem}"):
        return candidate
    return None


def resolve_object_path(unit_stem: str) -> str:
    """Resolve the ninja build target for a unit."""
    # Convention: build/GCBE7D/src/<stem>.o
    stem = unit_stem.replace(".c", "").replace(".s", "")
    return f"build/{DEFAULT_VERSION}/src/{stem}.o"


def main() -> int:
    parser = argparse.ArgumentParser(description="Sweep compiler flags for matching optimization")
    parser.add_argument("unit", help="Unit file name (e.g. camera.c)")
    parser.add_argument("--function", help="Specific function to track (e.g. MoveGameCamera)")
    parser.add_argument("--flags", help="Comma-separated flags to test (overrides defaults)")
    parser.add_argument("--combos", type=int, default=1,
                        help="Max combination size to test (1=single flags, 2=pairs, etc.)")
    parser.add_argument("--version", default=DEFAULT_VERSION)
    parser.add_argument("--toolchain", default=DEFAULT_TOOLCHAIN)
    parser.add_argument("--top", type=int, default=10, help="Show top N results")
    parser.add_argument("--dry-run", action="store_true", help="Show what would be tested")
    args = parser.parse_args()

    unit_stem = args.unit
    if "/" in unit_stem or "\\" in unit_stem:
        unit_stem = Path(unit_stem).name

    # Resolve paths
    source_path = resolve_source_path(unit_stem)
    object_path = resolve_object_path(unit_stem)

    if source_path is None:
        print(f"Error: Could not find source for {unit_stem}", file=sys.stderr)
        return 1

    # Parse flags to test
    if args.flags:
        flags_to_test = [f.strip() for f in args.flags.split(",")]
    else:
        flags_to_test = DEFAULT_FLAGS

    # Generate all combinations
    combos: list[tuple[str, ...]] = []
    for size in range(1, args.combos + 1):
        combos.extend(itertools.combinations(flags_to_test, size))

    print(f"Unit: {unit_stem}")
    print(f"Source: {source_path}")
    print(f"Object: {object_path}")
    print(f"Function: {args.function or '(all)'}")
    print(f"Flags to test: {len(flags_to_test)}")
    print(f"Combinations: {len(combos)}")
    print()

    if args.dry_run:
        for combo in combos:
            print(f"  Would test: {list(combo)}")
        return 0

    # Get baseline match
    print("Getting baseline (no extra flags)...")
    baseline = get_match_percent(unit_stem, args.function)
    print(f"Baseline match: {baseline:.2f}%" if baseline else "Baseline: unknown")
    print()

    # Save original configure.py
    original_configure = CONFIGURE_PY.read_text(encoding="utf-8")

    results: list[tuple[float, list[str]]] = []

    try:
        for i, combo in enumerate(combos, 1):
            flags = list(combo)
            label = " ".join(flags)
            print(f"[{i}/{len(combos)}] Testing: {label} ...", end=" ", flush=True)

            try:
                # Modify configure.py
                modify_configure(unit_stem, flags)

                # Reconfigure
                if not run_configure(args.version, args.toolchain):
                    print("CONFIGURE FAILED")
                    restore_configure(original_configure)
                    continue

                # Touch source to force rebuild
                touch_source(source_path)

                # Build
                if not build_object(object_path):
                    print("BUILD FAILED")
                    restore_configure(original_configure)
                    # Reconfigure with original to restore build.ninja
                    run_configure(args.version, args.toolchain)
                    continue

                # Check match
                match = get_match_percent(unit_stem, args.function)
                if match is not None:
                    delta = match - (baseline or 0)
                    marker = "▲" if delta > 0.01 else ("▼" if delta < -0.01 else "=")
                    print(f"{match:.2f}% ({marker}{abs(delta):+.2f}%)")
                    results.append((match, flags))
                else:
                    print("NO DATA")
            finally:
                # Restore configure.py after each test
                restore_configure(original_configure)

    except KeyboardInterrupt:
        print("\n\nInterrupted! Restoring configure.py...")
    finally:
        # Always restore
        restore_configure(original_configure)
        # Reconfigure to restore build.ninja
        run_configure(args.version, args.toolchain)

    # Sort and display results
    if not results:
        print("\nNo results collected.")
        return 1

    results.sort(key=lambda x: x[0], reverse=True)
    print(f"\n{'=' * 60}")
    print(f"Top {args.top} results (baseline: {baseline:.2f}%):")
    print(f"{'=' * 60}")
    for i, (match, flags) in enumerate(results[:args.top], 1):
        delta = match - (baseline or 0)
        marker = "▲" if delta > 0.01 else ("▼" if delta < -0.01 else "=")
        print(f"  {i:2d}. {match:6.2f}% ({marker}{abs(delta):+.2f}%)  {' '.join(flags)}")

    # Show best combination
    best_match, best_flags = results[0]
    if best_match > (baseline or 0) + 0.01:
        print(f"\n★ Best improvement: {' '.join(best_flags)}")
        print(f"  Apply with: extra_cflags={json.dumps(best_flags)}")
    else:
        print(f"\nNo improvement found over baseline ({baseline:.2f}%)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
