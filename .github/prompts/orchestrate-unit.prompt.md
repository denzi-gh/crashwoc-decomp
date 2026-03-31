# Orchestrate One Unit

Use this prompt when you want GitHub Copilot to keep pushing one file or unit forward with sub-agent style orchestration.

Target unit or file: `<replace with src/... path or unit name>`

You are an autonomous orchestration agent for Crash Bandicoot: The Wrath of Cortex (GameCube).

First read:

- `AGENTS.md`
- `.github/copilot-instructions.md`
- `.github/instructions/decomp.instructions.md`
- `.github/instructions/ai-tooling.instructions.md`

Workflow:

1. Start with `python tools/ai_match_plan.py <target-file-or-unit>`.
2. Keep one active function selected at a time.
3. Use helper scripts first: `ai_lookup_symbol.py`, `ai_lookup_unit.py`, `ai_context.py`, `ai_ghidra.py`.
4. Use read-only exploration and context gathering freely, but keep exactly one writing flow active at a time.
5. Build `ninja build/GCBE7D/src/<unit>.ctx` when source context is thin.
6. Edit only the active function, then build `ninja build/GCBE7D/src/<unit>.o`.
7. Re-check with `python tools/ai_match_plan.py <target-file-or-unit>`.
8. If a function stops improving after 4 build/measure cycles, note the blocker and move to the next function.
9. Repeat until every reachable function is either 100% matched or clearly blocked.

Critical rules:

- Never run interactive `objdiff-cli diff` sessions.
- No inline asm.
- Prefer helper scripts over manual repo-wide searching when a helper already exists.
- Use Ghidra only as supporting context; validate names and ownership with local repo data.
- Keep progress measurable: build, inspect, re-check, repeat.

Report back with:

- functions fully matched
- functions skipped or blocked
- final active target or next recommended target
- commands used for final verification
