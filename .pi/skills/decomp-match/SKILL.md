---
name: decomp-match
description: Autonomous matching-decompilation workflow for Crash WOC GameCube units. Use when the agent needs to decomp-match functions in a C source file.
---

# Decomp Match Skill

You are a decompilation agent for Crash Bandicoot: The Wrath of Cortex (GameCube).
The compiler is ProDG 3.5 (gcc 2.95.2). The target is `GCBE7D`.

## Before You Start

Read these repo docs (they contain critical rules):
- `AGENTS.md`

## Core Loop

1. **Get your assignment**: The dashboard provides a unit name. Run:
   ```
   python tools/ai_match_plan.py <unit>
   ```
2. **Pick the next unmatched function** from the output.
3. **Gather context**:
   ```
   python tools/ai_context.py <function_name>
   python tools/ai_lookup_symbol.py <function_name>
   ```
4. **Inspect target assembly**: `build/GCBE7D/asm/<unit>.s`
5. **Edit only one function** in the source file.
6. **Build only the single object**:
   ```
   ninja build/GCBE7D/src/<unit>.o
   ```
7. **Check progress**:
   ```
   python tools/ai_match_plan.py <unit>
   ```
8. **If stuck for 4 cycles**, note the blocker and move to the next function.
9. **Repeat** until all functions are matched or blocked.

## Available Helper Scripts

| Script | Purpose |
|--------|---------|
| `python tools/ai_match_plan.py <unit>` | Ranked function backlog for unit |
| `python tools/ai_context.py <query>` | One-stop context summary |
| `python tools/ai_lookup_symbol.py <name>` | Symbol lookup by name or address |
| `python tools/ai_lookup_unit.py <unit>` | Unit metadata and paths |
| `python tools/ai_diff.py <unit>` | Function diff summary without interactive objdiff |
| `python tools/ai_status.py` | Project-wide progress overview |
| `python tools/ai_ghidra.py --project <path> decompile <addr>` | Ghidra decompile (if configured) |
| `python tools/ai_ghidra.py --project <path> disasm <addr>` | Ghidra disassembly (if configured) |
| `python tools/ai_ghidra.py --project <path> find-function <name>` | Ghidra function search |

## Critical Rules

- **One function at a time.** Edit, build, check, repeat.
- **No inline asm.** Always express the target in C code.
- **No interactive objdiff.** Use `python tools/ai_diff.py` or `ninja changes`.
- **No `#pragma once`** in `.c` files.
- **Don't edit generated files**: `build.ninja`, `objdiff.json`, `compile_commands.json`, `config/GCBE7D/splits.generated.txt`, `config/GCBE7D/config.generated.yml`.
- **DWARF is hints only.** `config/GCBE7D/symbols.txt` is ground truth.
- **Prefer true matches**: typed fields, structured control flow, proper structs — not pointer arithmetic or goto-heavy code.
- **Build single object** during iteration: `ninja build/GCBE7D/src/<unit>.o` — not full `ninja`.

## When Done

Report:
- Functions fully matched (100%)
- Functions blocked (with reason)
- Final verification command used
