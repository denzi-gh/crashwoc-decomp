---
name: match-workflow
description: Drive symbol-to-verification matching work in crashwoc-decomp. Use when Codex needs to take a symbol or unit from lookup through context gathering, narrow local builds, and final regression checks.
---

# Match Workflow

## Standard Loop

1. Resolve the target with `python tools/ai_lookup_symbol.py ...`, `python tools/ai_lookup_unit.py ...`, or `python tools/ai_context.py ...`.
2. If the task starts from a file or unit, run `python tools/ai_match_plan.py <unit-or-path>` to list the remaining functions and pick the next target.
3. Build the context target with `ninja build/GCBE7D/src/<unit>.ctx` when you need flattened includes for decomp work.
4. Edit the source file with the smallest viable change.
5. Build the single object with `ninja build/GCBE7D/src/<unit>.o`.
6. Re-run `python tools/ai_lookup_symbol.py <symbol>`, `python tools/ai_context.py <symbol>`, or `python tools/ai_match_plan.py <unit>` to confirm the target still resolves and to inspect updated match data.
7. Run `ninja changes` if the work can affect matching or regression state. If `build/GCBE7D/baseline.json` is missing, run `ninja baseline` first.

## Practical Rules

- Prefer the normalized unit name printed by the lookup helpers when duplicate basenames exist.
- Use `build/GCBE7D/report.json` through `ai_context.py` when you need function-level fuzzy-match data.
- For regression checks in automation, use `ninja changes` and `python tools/changes_fmt.py build/GCBE7D/report_changes.json`; avoid interactive `objdiff-cli diff` sessions.
- Use `build/GCBE7D/asm/<unit>.s` when context and current source are not enough to reconstruct the exact function shape.
- Keep code edits narrow and reversible. Avoid unrelated cleanup while matching.
- If a function stops improving after 4 build/measure cycles, record the blocker and move to the next function instead of stalling.

## References

- Read [references/verification.md](references/verification.md) for the default verification sequence and exit criteria.
- Read [references/file-campaign.md](references/file-campaign.md) for file-based autonomous matching loops.
- Read [references/match-style.md](references/match-style.md) for high-signal style rules that improve true matches.
