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
5. Run `ninja changes` if the work can affect matching or regression state.

## Practical Rules

- Prefer the normalized unit name printed by the lookup helpers when duplicate basenames exist.
- Use `build/GCBE7D/report.json` through `ai_context.py` when you need function-level fuzzy-match data.
- Keep code edits narrow and reversible. Avoid unrelated cleanup while matching.

## References

- Read [references/verification.md](references/verification.md) for the default verification sequence and exit criteria.
