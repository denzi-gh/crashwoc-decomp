---
name: match-workflow
description: Drive symbol-to-verification matching work in crashwoc-decomp. Use when Codex needs to take a symbol or unit from lookup through context gathering, narrow local builds, and final regression checks.
---

# Match Workflow

## Standard Loop

1. Resolve the target with `python tools/ai_lookup_symbol.py ...`, `python tools/ai_lookup_unit.py ...`, or `python tools/ai_context.py ...`.
2. Build the context target with `ninja build/GCBE7D/src/<unit>.ctx` when you need flattened includes for decomp work.
3. Edit the source file with the smallest viable change.
4. Build the single object with `ninja build/GCBE7D/src/<unit>.o`.
5. Re-run `python tools/ai_lookup_symbol.py <symbol>` or `python tools/ai_context.py <symbol>` to confirm the target still resolves and to inspect updated match data.
6. Run `ninja changes` if the work can affect matching or regression state. If `build/GCBE7D/baseline.json` is missing, run `ninja baseline` first.

## Practical Rules

- Prefer the normalized unit name printed by the lookup helpers when duplicate basenames exist.
- Use `build/GCBE7D/report.json` through `ai_context.py` when you need function-level fuzzy-match data.
- For regression checks in automation, use `ninja changes` and `python tools/changes_fmt.py build/GCBE7D/report_changes.json`; avoid interactive `objdiff-cli diff` sessions.
- Use `build/GCBE7D/asm/<unit>.s` when context and current source are not enough to reconstruct the exact function shape.
- Keep code edits narrow and reversible. Avoid unrelated cleanup while matching.

## References

- Read [references/verification.md](references/verification.md) for the default verification sequence and exit criteria.
