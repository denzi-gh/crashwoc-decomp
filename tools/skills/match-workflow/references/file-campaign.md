# File Campaign

Use this workflow when the user hands over a source file or unit and expects sustained autonomous progress:

1. Run `python tools/ai_match_plan.py <unit-or-path>`.
2. Pick the next unmatched function from the plan.
3. Build `ninja build/GCBE7D/src/<unit>.ctx` if you need flattened includes.
4. Inspect `build/GCBE7D/asm/<unit>.s` when source shape, nearby constants, or rodata are unclear.
5. Edit only the active function when possible.
6. Build `ninja build/GCBE7D/src/<unit>.o`.
7. Re-run `python tools/ai_lookup_symbol.py <symbol>`, `python tools/ai_context.py <symbol>`, or `python tools/ai_match_plan.py <unit>`.
8. Repeat until the function reaches 100% or stops improving after 4 build/measure cycles.
9. Move to the next remaining function instead of stalling.

## Anti-Stall Rules

- Do not run interactive `objdiff-cli diff` commands in an automated loop.
- Do not sit in a long reasoning loop when a single-object build, report refresh, or extracted asm lookup can answer the next question.
- Keep a concrete next function selected at all times.
- Treat a fresh `ninja baseline` only as a prerequisite for `ninja changes`, not as proof that a change helped.
