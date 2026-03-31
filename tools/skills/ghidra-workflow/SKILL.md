---
name: ghidra-workflow
description: Query the local Crash WOC Ghidra project through analyzeHeadless.bat for function lookup, decompilation, disassembly, and datatype inspection.
---

# Ghidra Workflow

## Purpose

Use this skill when you need original-side reverse-engineering context that is easier to inspect through Ghidra than through raw asm alone.

## Configuration

`python tools/ai_ghidra.py` needs a local Ghidra project and installation.

Pass configuration explicitly:

- `--project <path-to-project-dir-or-.gpr>`
- `--ghidra-home <path-to-ghidra-install-root>` when it cannot be inferred
- `--program <project-pathname-or-program-name>` when the project contains multiple programs

Or set environment variables:

- `CRASHWOC_GHIDRA_PROJECT`
- `CRASHWOC_GHIDRA_HOME` or `GHIDRA_HOME`
- `CRASHWOC_GHIDRA_PROGRAM`

If the project directory already lives under the Ghidra install root and contains exactly one `.gpr`, `ai_ghidra.py` can infer most of this automatically.

## Read-Only Commands

- `python tools/ai_ghidra.py --project <path> programs`
- `python tools/ai_ghidra.py --project <path> --program <program> find-function MoveCrate`
- `python tools/ai_ghidra.py --project <path> --program <program> decompile 0x800032A0`
- `python tools/ai_ghidra.py --project <path> --program <program> disasm 0x800032A0 -n 30`
- `python tools/ai_ghidra.py --project <path> --program <program> type-get crate`

## Practical Rules

- Treat Ghidra output as supporting evidence, not automatically trusted truth.
- Cross-check names, ownership, and addresses with local repo data first.
- Prefer extracted asm in `build/GCBE7D/asm/` for final instruction-level confirmation.
- Use Ghidra to clarify function structure, nearby helpers, and datatype hints.
- The headless analyzer may fail if the same project is open elsewhere; close GUI sessions first if needed.

## Recommended Pairings

- `python tools/ai_context.py <symbol-or-unit>`
- `python tools/ai_lookup_symbol.py <symbol-or-address>`
- `python tools/ai_lookup_unit.py <unit-or-address>`
- `python tools/ai_match_plan.py <unit-or-path>`

## Output Modes

- default text output is concise for terminal use
- `--json` is available for automation and script composition

## References

- Read [../symbol-lookup/SKILL.md](../symbol-lookup/SKILL.md) for repo-first lookup rules.
- Read [../match-workflow/SKILL.md](../match-workflow/SKILL.md) for how to fold Ghidra findings into matching work.
