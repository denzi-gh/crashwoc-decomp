#!/usr/bin/env python3
"""Variable declaration order permuter for register allocation matching.

GCC 2.95.2 uses declaration order as a tiebreaker in register allocation.
This tool systematically reorders variable declarations in a function and
tests each permutation to find the order that produces the best match.

Usage:
    python tools/ai_decl_permute.py camera.c MoveGameCamera
    python tools/ai_decl_permute.py camera.c MoveGameCamera --target-order "vehicle,aySEEK,xrot_override"
    python tools/ai_decl_permute.py camera.c MoveGameCamera --analyze
    python tools/ai_decl_permute.py camera.c MoveGameCamera --max-perms 100
"""
from __future__ import annotations

import argparse
import itertools
import json
import os
import re
import subprocess
import sys
import time
from copy import deepcopy
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "GCBE7D"

# Variables known to be assigned to callee-saved registers in MoveGameCamera
# Format: (variable_name, target_register, description)
KNOWN_CALLEE_SAVED = [
    ("vehicle", "r15", "vehicle type ID"),
    ("aySEEK", "r16", "Y-axis seek mode"),
    ("xrot_override", "r17", "X rotation override"),
    ("zrot_override", "r18", "Z rotation override"),
    ("yrot_override", "r19", "Y rotation override"),
    ("oldmode", "r21", "previous camera mode"),
    ("obj", "r25", "object pointer"),
    ("GameCamera", "r27", "camera struct pointer"),
]


@dataclass
class Declaration:
    """A variable declaration extracted from source."""
    line_number: int
    text: str
    var_name: str
    var_type: str
    is_callee_saved: bool = False
    target_register: str = ""

    def __repr__(self) -> str:
        reg = f" ({self.target_register})" if self.target_register else ""
        return f"{self.var_name}{reg}: {self.text.strip()}"


def extract_declarations(source_lines: list[str], func_name: str) -> tuple[int, int, list[Declaration]]:
    """Extract variable declarations from a function.

    Returns (decl_start_line, decl_end_line, list_of_declarations).
    Line numbers are 0-indexed.
    """
    in_func = False
    brace_depth = 0
    func_start = -1
    decl_start = -1
    decl_end = -1
    declarations: list[Declaration] = []

    for i, line in enumerate(source_lines):
        stripped = line.strip()

        # Find function definition - look for the function name with opening paren
        if not in_func:
            if func_name in stripped and '(' in stripped:
                # Verify it's actually a function definition (not a call)
                # Look for return type before the name
                fn_pattern = rf'(?:void|int|float|char|short|long|unsigned|signed|struct\s+\w+\s*\*?)\s+{re.escape(func_name)}\s*\('
                if re.search(fn_pattern, stripped):
                    in_func = True
                    # Scan forward to opening brace
                    for j in range(i, min(i + 10, len(source_lines))):
                        if '{' in source_lines[j]:
                            func_start = j
                            brace_depth = source_lines[j].count('{') - source_lines[j].count('}')
                            break
            continue

        if not in_func or func_start < 0:
            continue

        # Skip the function start line itself (braces already counted)
        if i == func_start:
            continue

        # Track braces (count all { and } in this line)
        for ch in stripped:
            if ch == '{':
                brace_depth += 1
            elif ch == '}':
                brace_depth -= 1

        if brace_depth <= 0:
            break  # End of function

        # Only look at top-level declarations (brace_depth == 1)
        if brace_depth != 1:
            continue

        # Skip empty lines and comments
        if not stripped or stripped.startswith("//") or stripped.startswith("/*"):
            if decl_start < 0:
                continue
            # Allow blank lines and comments inside declaration block
            continue

        # Detect variable declarations
        # Patterns: type name; | type *name; | struct type *name; | type name = expr;
        is_decl = False
        var_name = ""
        var_type = ""

        # Pattern 1: struct/enum type with optional pointer(s)
        m = re.match(
            r'^((?:(?:unsigned|signed|volatile|const|static|register|extern)\s+)*'
            r'(?:struct|enum)\s+\w+)\s*(\*?\s*\*?)\s*'
            r'(\w+)\s*[;=\[]',
            stripped,
        )
        if m:
            var_type = (m.group(1).strip() + " " + m.group(2).strip()).strip()
            var_name = m.group(3)
            is_decl = True

        # Pattern 2: basic type with optional pointer(s)
        if not is_decl:
            m = re.match(
                r'^((?:(?:unsigned|signed|volatile|const|static|register|extern)\s+)*'
                r'(?:int|short|long\s*long|long|char|float|double|void|u8|u16|u32|s8|s16|s32|f32|f64))\s*(\*?\s*\*?)\s*'
                r'(\w+)\s*[;=\[]',
                stripped,
            )
            if m:
                var_type = (m.group(1).strip() + " " + m.group(2).strip()).strip()
                var_name = m.group(3)
                is_decl = True

        # Pattern 3: typedef name (word_t pattern) with optional pointer
        if not is_decl:
            m = re.match(
                r'^((?:(?:unsigned|signed|volatile|const|static|register|extern)\s+)*'
                r'\w+_[st])\s*(\*?\s*\*?)\s*'
                r'(\w+)\s*[;=\[]',
                stripped,
            )
            if m:
                var_type = (m.group(1).strip() + " " + m.group(2).strip()).strip()
                var_name = m.group(3)
                is_decl = True

        # Exclude false positives
        if is_decl and var_type.split()[0] in ("if", "for", "while", "switch", "return", "case", "goto", "else", "break"):
            is_decl = False

        if is_decl and var_name:
            if decl_start < 0:
                decl_start = i
            decl_end = i

            # Check if this is a known callee-saved variable
            target_reg = ""
            is_cs = False
            for known_name, reg, _ in KNOWN_CALLEE_SAVED:
                if var_name == known_name:
                    target_reg = reg
                    is_cs = True
                    break

            declarations.append(Declaration(
                line_number=i,
                text=line,
                var_name=var_name,
                var_type=var_type,
                is_callee_saved=is_cs,
                target_register=target_reg,
            ))
        elif stripped and not stripped.startswith("//") and not stripped.startswith("/*"):
            # Non-declaration non-comment line means end of declaration block
            if decl_start >= 0:
                break

    return decl_start, decl_end, declarations


def apply_permutation(source_lines: list[str], declarations: list[Declaration],
                      new_order: list[int]) -> list[str]:
    """Apply a declaration permutation to the source.

    new_order[i] = index into original declarations list for position i.
    """
    result = list(source_lines)

    # Get the original line numbers and new declaration texts
    original_lines = [d.line_number for d in declarations]
    reordered_decls = [declarations[idx] for idx in new_order]

    # Replace each line
    for i, line_num in enumerate(original_lines):
        if i < len(reordered_decls):
            # Preserve indentation from original line
            indent = len(source_lines[line_num]) - len(source_lines[line_num].lstrip())
            new_text = " " * indent + reordered_decls[i].text.strip() + "\n"
            result[line_num] = new_text

    return result


def build_and_check(source_path: Path, source_lines: list[str],
                    unit_stem: str, function_name: str | None) -> float | None:
    """Write source, build, and return match percentage."""
    # Write modified source
    source_path.write_text("".join(source_lines), encoding="utf-8")

    # Touch to force rebuild
    source_path.touch()

    # Build
    obj_target = f"build/{DEFAULT_VERSION}/src/{unit_stem}.o"
    result = subprocess.run(
        ["ninja", obj_target],
        cwd=str(ROOT),
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        return None

    # Read match from report
    report_path = ROOT / "build" / DEFAULT_VERSION / "report.json"
    if not report_path.is_file():
        return None

    with open(report_path, "r") as f:
        report = json.load(f)

    for unit in report.get("units", []):
        if unit_stem.replace(".c", "") not in unit.get("name", ""):
            continue
        for fn in unit.get("functions", []):
            if function_name and fn.get("name") != function_name:
                continue
            return fn.get("fuzzy_match_percent")
    return None


def smart_permutations(declarations: list[Declaration],
                       target_order: list[str] | None = None,
                       max_perms: int = 100) -> list[list[int]]:
    """Generate smart permutations to test.

    Strategy:
    1. If target_order given, try that first
    2. Try swapping adjacent callee-saved variables
    3. Try moving callee-saved variables to match target register order
    4. Random permutations of callee-saved vars only (non-CS stay in place)
    """
    n = len(declarations)
    perms: list[list[int]] = []
    seen: set[tuple[int, ...]] = set()

    identity = list(range(n))
    seen.add(tuple(identity))

    # Find callee-saved variable indices
    cs_indices = [i for i, d in enumerate(declarations) if d.is_callee_saved]

    # Strategy 1: Target register order
    if target_order:
        name_to_idx = {d.var_name: i for i, d in enumerate(declarations)}
        new_cs_order = []
        for name in target_order:
            if name in name_to_idx and name_to_idx[name] in cs_indices:
                new_cs_order.append(name_to_idx[name])
        # Fill remaining CS vars
        for idx in cs_indices:
            if idx not in new_cs_order:
                new_cs_order.append(idx)

        if len(new_cs_order) == len(cs_indices):
            perm = list(identity)
            for pos, orig_idx in zip(cs_indices, new_cs_order):
                perm[pos] = orig_idx
            key = tuple(perm)
            if key not in seen:
                perms.append(perm)
                seen.add(key)

    # Strategy 2: Adjacent swaps of all declarations
    for i in range(n - 1):
        perm = list(identity)
        perm[i], perm[i + 1] = perm[i + 1], perm[i]
        key = tuple(perm)
        if key not in seen:
            perms.append(perm)
            seen.add(key)

    # Strategy 3: All permutations of callee-saved vars (if not too many)
    if len(cs_indices) <= 6:
        for cs_perm in itertools.permutations(cs_indices):
            perm = list(identity)
            for pos, orig_idx in zip(cs_indices, cs_perm):
                perm[pos] = orig_idx
            key = tuple(perm)
            if key not in seen and len(perms) < max_perms:
                perms.append(perm)
                seen.add(key)
    else:
        # Too many CS vars for exhaustive search — try targeted swaps
        for i in range(len(cs_indices)):
            for j in range(i + 1, len(cs_indices)):
                perm = list(identity)
                perm[cs_indices[i]], perm[cs_indices[j]] = perm[cs_indices[j]], perm[cs_indices[i]]
                key = tuple(perm)
                if key not in seen and len(perms) < max_perms:
                    perms.append(perm)
                    seen.add(key)

    # Strategy 4: Move groups up or down
    for i in range(n):
        # Move declaration i to position 0
        if i > 0:
            perm = list(identity)
            val = perm.pop(i)
            perm.insert(0, val)
            key = tuple(perm)
            if key not in seen and len(perms) < max_perms:
                perms.append(perm)
                seen.add(key)
        # Move declaration i to end
        if i < n - 1:
            perm = list(identity)
            val = perm.pop(i)
            perm.append(val)
            key = tuple(perm)
            if key not in seen and len(perms) < max_perms:
                perms.append(perm)
                seen.add(key)

    return perms[:max_perms]


def main() -> int:
    parser = argparse.ArgumentParser(description="Variable declaration order permuter")
    parser.add_argument("unit", help="Unit file name (e.g. camera.c)")
    parser.add_argument("function", help="Function name (e.g. MoveGameCamera)")
    parser.add_argument("--target-order", help="Comma-separated target variable order")
    parser.add_argument("--max-perms", type=int, default=50, help="Max permutations to test")
    parser.add_argument("--analyze", action="store_true", help="Only analyze declarations, don't permute")
    parser.add_argument("--dry-run", action="store_true", help="Show permutations without building")
    parser.add_argument("--version", default=DEFAULT_VERSION)
    args = parser.parse_args()

    unit_stem = Path(args.unit).stem
    source_name = Path(args.unit).name
    if not source_name.endswith(".c"):
        source_name += ".c"

    # Find source file
    source_path = None
    for candidate in ROOT.rglob(f"src/**/{source_name}"):
        source_path = candidate
        break
    if source_path is None:
        print(f"Error: Source file not found for {source_name}", file=sys.stderr)
        return 1

    # Read source
    source_lines = source_path.read_text(encoding="utf-8").splitlines(keepends=True)

    # Extract declarations
    decl_start, decl_end, declarations = extract_declarations(source_lines, args.function)

    print(f"Source: {source_path}")
    print(f"Function: {args.function}")
    print(f"Declaration block: lines {decl_start + 1}-{decl_end + 1}")
    print(f"Declarations found: {len(declarations)}")
    print()

    for i, d in enumerate(declarations):
        cs_marker = f" ← {d.target_register}" if d.target_register else ""
        print(f"  [{i:2d}] {d.var_name:25s} {d.var_type:20s} (line {d.line_number + 1}){cs_marker}")

    if args.analyze:
        print(f"\nCallee-saved variables: {sum(1 for d in declarations if d.is_callee_saved)}")
        print("Target register order:")
        for name, reg, desc in KNOWN_CALLEE_SAVED:
            found = any(d.var_name == name for d in declarations)
            marker = "✓" if found else "✗"
            print(f"  {marker} {reg}: {name} ({desc})")
        return 0

    # Generate permutations
    target_order = None
    if args.target_order:
        target_order = [s.strip() for s in args.target_order.split(",")]

    perms = smart_permutations(declarations, target_order, args.max_perms)
    print(f"\nPermutations to test: {len(perms)}")

    if args.dry_run:
        for i, perm in enumerate(perms[:20]):
            names = [declarations[idx].var_name for idx in perm]
            print(f"  {i + 1}. {', '.join(names[:8])}...")
        return 0

    # Save original source
    original_source = source_path.read_text(encoding="utf-8")
    original_lines = original_source.splitlines(keepends=True)

    # Get baseline
    print("\nGetting baseline...")
    baseline = build_and_check(source_path, original_lines, unit_stem, args.function)
    print(f"Baseline: {baseline:.2f}%" if baseline else "Baseline: unknown")

    results: list[tuple[float, list[int], str]] = []

    try:
        for i, perm in enumerate(perms, 1):
            names = [declarations[idx].var_name for idx in perm]
            label = ", ".join(names[:5]) + ("..." if len(names) > 5 else "")
            print(f"[{i}/{len(perms)}] Testing: {label} ...", end=" ", flush=True)

            modified = apply_permutation(original_lines, declarations, perm)
            match = build_and_check(source_path, modified, unit_stem, args.function)

            if match is not None:
                delta = match - (baseline or 0)
                marker = "▲" if delta > 0.01 else ("▼" if delta < -0.01 else "=")
                print(f"{match:.2f}% ({marker}{abs(delta):+.2f}%)")
                results.append((match, perm, label))
            else:
                print("BUILD FAILED")

    except KeyboardInterrupt:
        print("\n\nInterrupted!")
    finally:
        # Always restore original
        source_path.write_text(original_source, encoding="utf-8")
        print(f"\nRestored original source.")

    # Results
    if not results:
        print("No results collected.")
        return 1

    results.sort(key=lambda x: x[0], reverse=True)
    print(f"\n{'=' * 60}")
    print(f"Top results (baseline: {baseline:.2f}%):")
    print(f"{'=' * 60}")
    for i, (match, perm, label) in enumerate(results[:10], 1):
        delta = match - (baseline or 0)
        marker = "▲" if delta > 0.01 else ("▼" if delta < -0.01 else "=")
        print(f"  {i:2d}. {match:6.2f}% ({marker}{abs(delta):+.2f}%)  {label}")

    best_match, best_perm, _ = results[0]
    if best_match > (baseline or 0) + 0.01:
        print(f"\n★ Best order:")
        for idx in best_perm:
            d = declarations[idx]
            print(f"    {d.text.strip()}")
    else:
        print(f"\nNo improvement found over baseline ({baseline:.2f}%)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
