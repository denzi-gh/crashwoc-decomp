#!/usr/bin/env python3
"""Instruction-level diff aligner for decompilation matching.

Aligns compiled vs target PowerPC instruction streams using sequence alignment,
then classifies each pair as: exact match, register swap, opcode match, or gap.

Usage:
    python tools/ai_instr_align.py camera.c MoveGameCamera
    python tools/ai_instr_align.py camera.c MoveGameCamera --context 3
    python tools/ai_instr_align.py camera.c MoveGameCamera --summary
    python tools/ai_instr_align.py camera.c MoveGameCamera --region 100-200
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "GCBE7D"
OBJDUMP = None

# Find objdump
for candidate in [
    Path(r"C:\devkitPro\devkitPPC\bin\powerpc-eabi-objdump.exe"),
    Path("/opt/devkitpro/devkitPPC/bin/powerpc-eabi-objdump"),
    Path("/usr/bin/powerpc-linux-gnu-objdump"),
]:
    if candidate.is_file():
        OBJDUMP = str(candidate)
        break


def _normalize_operands(operands: str) -> str:
    """Normalize operand formatting for comparison.

    - Convert hex immediates to decimal: 0x190 -> 400
    - Remove spaces after commas: r1, r2 -> r1,r2
    - Normalize register names
    """
    # Remove spaces around commas
    operands = re.sub(r'\s*,\s*', ',', operands)

    # Convert hex immediates in memory operands: -0x190(r1) -> -400(r1)
    def hex_to_dec(m: re.Match) -> str:
        sign = m.group(1) or ""
        val = int(m.group(2), 16)
        rest = m.group(3)
        return f"{sign}{val}{rest}"

    operands = re.sub(r'(-?)0x([0-9A-Fa-f]+)(\([^)]+\))', hex_to_dec, operands)

    # Convert standalone hex immediates: 0x190 -> 400
    def standalone_hex(m: re.Match) -> str:
        sign = m.group(1) or ""
        val = int(m.group(2), 16)
        return f"{sign}{val}"

    operands = re.sub(r'(-?)0x([0-9A-Fa-f]+)(?![(\w])', standalone_hex, operands)

    return operands


@dataclass
class Instruction:
    offset: int          # byte offset from function start
    raw_hex: str         # raw hex bytes
    opcode: str          # e.g. "stw", "addi", "bl"
    operands: str        # e.g. "r3, 0x10(r1)"
    full_text: str       # complete line

    @property
    def registers(self) -> list[str]:
        return re.findall(r'\b(r\d+|f\d+|cr\d+)\b', self.operands)

    @property
    def normalized(self) -> str:
        """Opcode + operand structure with registers replaced by placeholders."""
        s = self.operands
        s = re.sub(r'\b(r|f|cr)\d+', r'\1?', s)
        return f"{self.opcode} {s}"


def parse_objdump_function(text: str, function_name: str) -> list[Instruction]:
    """Parse objdump output and extract instructions for a function."""
    instructions: list[Instruction] = []
    in_function = False
    func_base = 0

    for line in text.splitlines():
        # Detect function header: "00000abc <FunctionName>:" or similar
        m = re.match(r'^([0-9a-f]+)\s+<([^>]+)>:', line)
        if m:
            if m.group(2) == function_name or function_name in m.group(2):
                in_function = True
                func_base = int(m.group(1), 16)
            else:
                if in_function:
                    break  # End of our function
            continue

        if not in_function:
            continue

        # Empty line might signal end
        if not line.strip():
            if instructions:
                break
            continue

        # Parse instruction line: "    16e8:\t94 21 fe 70 \tstwu\tr1,-400(r1)"
        # objdump uses tabs between fields
        m = re.match(
            r'^\s+([0-9a-f]+):\s+'
            r'((?:[0-9a-f]{2}\s+)+)\s*'
            r'(\S+)'
            r'(?:\s+(.*))?$',
            line,
        )
        if m:
            offset = int(m.group(1), 16) - func_base
            raw_hex = m.group(2).strip()
            opcode = m.group(3)
            operands = (m.group(4) or "").strip()
            # Remove comments and relocation info
            operands = re.sub(r'\s*#.*$', '', operands)
            operands = re.sub(r'\s*<[^>]+>$', '', operands)
            # Normalize operands for comparison
            operands = _normalize_operands(operands)
            full_text = f"{offset:04x}: {opcode:10s} {operands}"
            instructions.append(Instruction(offset, raw_hex, opcode, operands, full_text))

    return instructions


def parse_target_asm(asm_path: Path, function_name: str) -> list[Instruction]:
    """Parse the target .s file for a function's instructions."""
    instructions: list[Instruction] = []
    in_function = False
    func_base = 0

    text = asm_path.read_text(encoding="utf-8", errors="replace")
    for line in text.splitlines():
        # Detect function: .fn MoveGameCamera, global
        if re.match(rf'\.fn\s+{re.escape(function_name)}\b', line):
            in_function = True
            continue
        if in_function and re.match(r'\.endfn\b', line):
            break
        if not in_function:
            continue

        # Skip labels, comments, directives
        stripped = line.strip()
        if not stripped or stripped.startswith("#") or stripped.startswith("."):
            continue
        if stripped.endswith(":"):
            continue

        # Parse: /* 8000B198 00008198  94 21 FE 70 */stwu r1, -0x190(r1)
        # The */ may be immediately followed by the opcode (no space)
        m = re.match(r'/\*\s*([0-9A-Fa-f]+)\s+[^*]*\*/\s*(\S+)\s*(.*)', stripped)
        if m:
            addr = int(m.group(1), 16)
            if not instructions:
                func_base = addr
            offset = addr - func_base
            opcode = m.group(2)
            operands = m.group(3).strip().rstrip(",")
            # Normalize operands: remove spaces after commas, normalize hex to decimal
            operands = _normalize_operands(operands)
            full_text = f"{offset:04x}: {opcode:10s} {operands}"
            instructions.append(Instruction(offset, "", opcode, operands, full_text))
            continue

        # Simpler format: just opcode operands (for manual asm)
        parts = stripped.split(None, 1)
        if parts and not parts[0].startswith(".") and not parts[0].endswith(":"):
            opcode = parts[0]
            operands = parts[1].strip().rstrip(",") if len(parts) > 1 else ""
            operands = _normalize_operands(operands)
            offset = len(instructions) * 4
            full_text = f"{offset:04x}: {opcode:10s} {operands}"
            instructions.append(Instruction(offset, "", opcode, operands, full_text))

    return instructions


def classify_pair(a: Instruction | None, b: Instruction | None) -> str:
    """Classify a pair of instructions."""
    if a is None and b is None:
        return "empty"
    if a is None:
        return "target_only"  # Missing in compiled
    if b is None:
        return "compiled_only"  # Extra in compiled
    if a.opcode == b.opcode and a.operands == b.operands:
        return "exact"
    if a.opcode == b.opcode:
        if a.normalized == b.normalized:
            return "regswap"
        return "opcode_match"
    return "mismatch"


def needleman_wunsch(compiled: list[Instruction], target: list[Instruction],
                     match_score: int = 2, mismatch_penalty: int = -1,
                     gap_penalty: int = -2) -> list[tuple[int | None, int | None]]:
    """Align two instruction sequences using Needleman-Wunsch algorithm.

    Returns a list of (compiled_idx, target_idx) pairs.
    None means a gap in that sequence.
    """
    n = len(compiled)
    m = len(target)

    # For very large functions, use a banded approach
    if n * m > 50_000_000:
        return _banded_align(compiled, target, bandwidth=500)

    # Score matrix
    score = [[0] * (m + 1) for _ in range(n + 1)]
    for i in range(1, n + 1):
        score[i][0] = score[i - 1][0] + gap_penalty
    for j in range(1, m + 1):
        score[0][j] = score[0][j - 1] + gap_penalty

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            a, b = compiled[i - 1], target[j - 1]
            if a.opcode == b.opcode and a.operands == b.operands:
                s = match_score + 1  # Bonus for exact match
            elif a.opcode == b.opcode:
                s = match_score
            else:
                s = mismatch_penalty
            score[i][j] = max(
                score[i - 1][j - 1] + s,
                score[i - 1][j] + gap_penalty,
                score[i][j - 1] + gap_penalty,
            )

    # Traceback
    alignment: list[tuple[int | None, int | None]] = []
    i, j = n, m
    while i > 0 or j > 0:
        if i > 0 and j > 0:
            a, b = compiled[i - 1], target[j - 1]
            if a.opcode == b.opcode and a.operands == b.operands:
                s = match_score + 1
            elif a.opcode == b.opcode:
                s = match_score
            else:
                s = mismatch_penalty
            if score[i][j] == score[i - 1][j - 1] + s:
                alignment.append((i - 1, j - 1))
                i -= 1
                j -= 1
                continue
        if i > 0 and score[i][j] == score[i - 1][j] + gap_penalty:
            alignment.append((i - 1, None))
            i -= 1
        else:
            alignment.append((None, j - 1))
            j -= 1

    alignment.reverse()
    return alignment


def _banded_align(compiled: list[Instruction], target: list[Instruction],
                  bandwidth: int = 500) -> list[tuple[int | None, int | None]]:
    """Banded alignment for very large sequences."""
    # Simple diagonal-following heuristic alignment
    alignment: list[tuple[int | None, int | None]] = []
    ci, ti = 0, 0
    n, m = len(compiled), len(target)

    while ci < n and ti < m:
        a, b = compiled[ci], target[ti]
        if a.opcode == b.opcode:
            alignment.append((ci, ti))
            ci += 1
            ti += 1
        else:
            # Look ahead in both sequences for a match
            best_ci, best_ti = -1, -1
            best_cost = bandwidth

            for look in range(1, min(50, n - ci, m - ti)):
                # Skip in compiled
                if ci + look < n and compiled[ci + look].opcode == target[ti].opcode:
                    if look < best_cost:
                        best_ci, best_ti = ci + look, ti
                        best_cost = look
                # Skip in target
                if ti + look < m and compiled[ci].opcode == target[ti + look].opcode:
                    if look < best_cost:
                        best_ci, best_ti = ci, ti + look
                        best_cost = look

            if best_ci >= 0:
                # Fill gaps
                while ci < best_ci:
                    alignment.append((ci, None))
                    ci += 1
                while ti < best_ti:
                    alignment.append((None, ti))
                    ti += 1
                alignment.append((ci, ti))
                ci += 1
                ti += 1
            else:
                alignment.append((ci, ti))
                ci += 1
                ti += 1

    while ci < n:
        alignment.append((ci, None))
        ci += 1
    while ti < m:
        alignment.append((None, ti))
        ti += 1

    return alignment


def format_alignment(compiled: list[Instruction], target: list[Instruction],
                     alignment: list[tuple[int | None, int | None]],
                     context: int = 0, region: tuple[int, int] | None = None) -> str:
    """Format the alignment as a readable diff."""
    lines: list[str] = []

    # Classify all pairs
    pairs: list[tuple[str, str, str, str]] = []  # (class, compiled_text, target_text, detail)
    for ci, ti in alignment:
        c_instr = compiled[ci] if ci is not None else None
        t_instr = target[ti] if ti is not None else None
        cls = classify_pair(c_instr, t_instr)

        c_text = c_instr.full_text if c_instr else ""
        t_text = t_instr.full_text if t_instr else ""

        detail = ""
        if cls == "regswap" and c_instr and t_instr:
            c_regs = c_instr.registers
            t_regs = t_instr.registers
            swaps = [(c, t) for c, t in zip(c_regs, t_regs) if c != t]
            if swaps:
                detail = "  regs: " + ", ".join(f"{c}→{t}" for c, t in swaps)

        pairs.append((cls, c_text, t_text, detail))

    # Filter to region if specified
    if region:
        start, end = region
        pairs = pairs[start:end]

    # Filter to non-exact with context
    if context >= 0:
        visible = [False] * len(pairs)
        for i, (cls, _, _, _) in enumerate(pairs):
            if cls != "exact":
                for j in range(max(0, i - context), min(len(pairs), i + context + 1)):
                    visible[j] = True

        filtered: list[tuple[str, str, str, str] | None] = []
        last_visible = False
        for i, (cls, c_text, t_text, detail) in enumerate(pairs):
            if visible[i]:
                filtered.append((cls, c_text, t_text, detail))
                last_visible = True
            else:
                if last_visible:
                    filtered.append(None)  # Separator
                last_visible = False
        pairs_to_show = filtered
    else:
        pairs_to_show = list(pairs)

    # Format output
    MARKERS = {
        "exact": " ",
        "regswap": "~",
        "opcode_match": "?",
        "mismatch": "!",
        "compiled_only": "+",
        "target_only": "-",
    }

    for entry in pairs_to_show:
        if entry is None:
            lines.append("  ···")
            continue
        cls, c_text, t_text, detail = entry
        marker = MARKERS.get(cls, "?")
        if cls in ("exact",):
            lines.append(f"{marker} {c_text}")
        elif cls == "compiled_only":
            lines.append(f"+ {c_text:44s}  |  {'':44s}  (extra in compiled)")
        elif cls == "target_only":
            lines.append(f"- {'':44s}  |  {t_text:44s}  (missing in compiled)")
        else:
            lines.append(f"{marker} {c_text:44s}  |  {t_text:44s}{detail}")

    return "\n".join(lines)


def compute_summary(compiled: list[Instruction], target: list[Instruction],
                    alignment: list[tuple[int | None, int | None]]) -> dict:
    """Compute summary statistics from alignment."""
    counts = {"exact": 0, "regswap": 0, "opcode_match": 0, "mismatch": 0,
              "compiled_only": 0, "target_only": 0}

    # Track register swap frequency
    reg_swaps: dict[tuple[str, str], int] = {}

    # Track mismatch regions
    mismatch_runs: list[tuple[int, int]] = []
    in_run = False
    run_start = 0

    for idx, (ci, ti) in enumerate(alignment):
        c_instr = compiled[ci] if ci is not None else None
        t_instr = target[ti] if ti is not None else None
        cls = classify_pair(c_instr, t_instr)
        counts[cls] += 1

        if cls == "regswap" and c_instr and t_instr:
            for c_reg, t_reg in zip(c_instr.registers, t_instr.registers):
                if c_reg != t_reg:
                    key = (c_reg, t_reg)
                    reg_swaps[key] = reg_swaps.get(key, 0) + 1

        is_diff = cls not in ("exact",)
        if is_diff and not in_run:
            run_start = idx
            in_run = True
        elif not is_diff and in_run:
            mismatch_runs.append((run_start, idx - 1))
            in_run = False

    if in_run:
        mismatch_runs.append((run_start, len(alignment) - 1))

    total = sum(counts.values())
    return {
        "total_aligned": total,
        "compiled_count": len(compiled),
        "target_count": len(target),
        "counts": counts,
        "match_pct": (counts["exact"] / total * 100) if total else 0,
        "regswap_pct": (counts["regswap"] / total * 100) if total else 0,
        "top_reg_swaps": sorted(reg_swaps.items(), key=lambda x: -x[1])[:15],
        "mismatch_regions": len(mismatch_runs),
        "largest_mismatch_run": max((e - s + 1 for s, e in mismatch_runs), default=0),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Instruction-level diff aligner")
    parser.add_argument("unit", help="Unit file name (e.g. camera.c)")
    parser.add_argument("function", help="Function name (e.g. MoveGameCamera)")
    parser.add_argument("--context", "-C", type=int, default=2,
                        help="Context lines around differences (-1 for all)")
    parser.add_argument("--summary", "-s", action="store_true",
                        help="Show only summary statistics")
    parser.add_argument("--region", help="Show only alignment region (e.g. 100-200)")
    parser.add_argument("--version", default=DEFAULT_VERSION)
    parser.add_argument("--output", "-o", help="Write output to file")
    args = parser.parse_args()

    unit_stem = Path(args.unit).stem

    # Get compiled instructions via objdump
    obj_path = ROOT / "build" / args.version / "src" / f"{unit_stem}.o"
    if not obj_path.is_file():
        print(f"Error: Object file not found: {obj_path}", file=sys.stderr)
        return 1

    if OBJDUMP is None:
        print("Error: powerpc-eabi-objdump not found", file=sys.stderr)
        return 1

    result = subprocess.run(
        [OBJDUMP, "-d", str(obj_path)],
        capture_output=True, text=True,
    )
    compiled = parse_objdump_function(result.stdout, args.function)

    # Get target instructions from asm file
    asm_path = ROOT / "build" / args.version / "asm" / f"{unit_stem}.s"
    if not asm_path.is_file():
        print(f"Error: Asm file not found: {asm_path}", file=sys.stderr)
        return 1

    target = parse_target_asm(asm_path, args.function)

    if not compiled:
        print(f"Error: No compiled instructions found for {args.function}", file=sys.stderr)
        return 1
    if not target:
        print(f"Error: No target instructions found for {args.function}", file=sys.stderr)
        return 1

    print(f"Compiled: {len(compiled)} instructions")
    print(f"Target:   {len(target)} instructions")
    print(f"Gap:      {len(compiled) - len(target):+d}")
    print()

    # Align
    print("Aligning instructions...", flush=True)
    alignment = needleman_wunsch(compiled, target)

    # Summary
    summary = compute_summary(compiled, target, alignment)
    print(f"Alignment: {summary['total_aligned']} pairs")
    print(f"  Exact matches:  {summary['counts']['exact']:4d} ({summary['match_pct']:.1f}%)")
    print(f"  Register swaps: {summary['counts']['regswap']:4d} ({summary['regswap_pct']:.1f}%)")
    print(f"  Opcode match:   {summary['counts']['opcode_match']:4d}")
    print(f"  Full mismatch:  {summary['counts']['mismatch']:4d}")
    print(f"  Extra compiled: {summary['counts']['compiled_only']:4d}")
    print(f"  Missing target: {summary['counts']['target_only']:4d}")
    print(f"  Mismatch regions: {summary['mismatch_regions']}")
    print(f"  Largest run:      {summary['largest_mismatch_run']} instructions")

    if summary["top_reg_swaps"]:
        print(f"\nTop register swaps:")
        for (c_reg, t_reg), count in summary["top_reg_swaps"]:
            print(f"  {c_reg:4s} → {t_reg:4s}  ×{count}")

    if args.summary:
        return 0

    # Full diff
    region = None
    if args.region:
        parts = args.region.split("-")
        region = (int(parts[0]), int(parts[1]))

    print()
    output = format_alignment(compiled, target, alignment,
                              context=args.context, region=region)

    if args.output:
        Path(args.output).write_text(output, encoding="utf-8")
        print(f"Written to {args.output}")
    else:
        print(output)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
