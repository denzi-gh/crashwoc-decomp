---
applyTo: "src/**/*.c,src/**/*.h,src/**/*.s"
---

# Decomp Source Instructions

See `AGENTS.md` for the full workflow, helper scripts, and repo map.

## Quick Reference

- Start with `python tools/ai_match_plan.py <unit-or-path>`.
- Build only the single object: `ninja build/GCBE7D/src/<unit>.o`.
- Build context: `ninja build/GCBE7D/src/<unit>.ctx`.
- Inspect target asm: `build/GCBE7D/asm/<unit>.s`.

## Critical Reminders

- **One function at a time.** Rebuild and re-check after each edit.
- **4-cycle limit**: If no improvement after 4 build/measure cycles, note the blocker and move on.
- **No interactive objdiff**: Use `objdiff-cli report generate` or `ninja changes`, never bare `objdiff-cli diff`.
- **No inline asm.** Keep decomp work in C.
- **Prefer true matches**: typed fields, structured control flow, proper structs — not pointer arithmetic or goto-heavy code.
- **Do not stop after analysis** if the file still has actionable functions to improve.
