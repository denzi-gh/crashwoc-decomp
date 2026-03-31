# Crash WOC Copilot Instructions

Crash Bandicoot: The Wrath of Cortex GameCube decompilation repo. The active target is `GCBE7D`, the default local toolchain is `prodg35`, and the compiler is ProDG 3.5.

## Start Here

Read `AGENTS.md` for the full repo guide: helper scripts, build commands, workflow, skills, and repo map.

## Critical Rules

These are the most common agent failure modes — duplicated here for visibility:

- **Use helper scripts first**: `ai_match_plan.py`, `ai_lookup_symbol.py`, `ai_lookup_unit.py`, `ai_context.py` — before grep or manual repo-wide searching.
- **Single-object builds**: `ninja build/GCBE7D/src/<unit>.o` during iteration, not full `ninja`.
- **No interactive objdiff**: Never run `objdiff-cli diff` without a symbol argument — it opens a live-refresh TUI that hangs agents. Use `objdiff-cli report generate` or `ninja changes` instead.
- **No inline asm**: Always write matching C code, even when matching is hard.
- **No `#pragma once` in C files.**
- **DWARF is fallback only**: `src/dump_alphaNGCport_DWARF.txt` is from an alpha build. Treat as hints, not ground truth. `config/GCBE7D/symbols.txt` is the validated source.
- **Do not hand-edit generated files**: `build.ninja`, `objdiff.json`, `compile_commands.json`, `config/GCBE7D/splits.generated.txt`, `config/GCBE7D/config.generated.yml`.
- **4-cycle stall limit**: If a function stops improving after 4 build/measure cycles, note the blocker and move to the next function.

- **No git commits** You are NOT ALLOWED to make git commits. Commits are only done by the user. You tend to overwrite work when trying to "save".

Everything else — autonomous workflow, matching style, anti-stall details, generated file list, configure commands — is in `AGENTS.md`.
