---
name: project-onboarding
description: Onboard Codex to crashwoc-decomp. Use when working in this repository and Codex needs the project layout, build and verification commands, generated-file rules, helper tooling, or the standard workflow for Crash Bandicoot: The Wrath of Cortex GameCube decompilation.
---

# Project Onboarding

## Quick Start

- Read [../../../AGENTS.md](../../../AGENTS.md) first for the repo-level summary.
- If generated build files may be stale or missing, run `python configure.py --version GCBE7D --toolchain prodg35`.
- Use `python tools/ai_lookup_symbol.py ...`, `python tools/ai_lookup_unit.py ...`, or `python tools/ai_context.py ...` before manual repo-wide searching.

## Build Flow

Use these commands as the default local workflow:

- `python configure.py --version GCBE7D --toolchain prodg35`
- `ninja`
- `ninja progress`
- `ninja baseline`
- `ninja changes`
- `python tools/changes_fmt.py build/GCBE7D/report_changes.json`

For automation, never use interactive `objdiff-cli diff` sessions. Prefer the
report-based commands above, or direct `objdiff-cli report changes ... -o <file>`
when you need raw objdiff output.

For narrow iteration, build the single object or context target instead of the whole project:

- `ninja build/GCBE7D/src/<unit>.o`
- `ninja build/GCBE7D/src/<unit>.ctx`

## Working Rules

- Edit checked-in source files such as `config/GCBE7D/splits.txt` and `config/GCBE7D/config.yml`, not generated outputs.
- Treat `config/GCBE7D/symbols.txt` as authoritative for validated symbol names.
- Treat `src/dump_alphaNGCport_DWARF.txt` as a clue source when validated data is missing.
- Prefer repo-local helper scripts over ad hoc parsing when a script already exists for the job.

## References

- Read [references/repo-map.md](references/repo-map.md) for the important project files and what they mean.
- Read [references/build-flow.md](references/build-flow.md) for configure, build, and regression commands.
