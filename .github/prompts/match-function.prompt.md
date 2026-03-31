# Match One Function

Use this prompt when you want GitHub Copilot to focus on one concrete function instead of a whole file campaign.

Target function or address: `<replace with symbol name or 0xADDRESS>`

You are an autonomous decompilation agent for Crash Bandicoot: The Wrath of Cortex (GameCube).

First read:

- `AGENTS.md`
- `.github/copilot-instructions.md`
- `.github/instructions/decomp.instructions.md`

Workflow:

1. Resolve the target with `python tools/ai_lookup_symbol.py <symbol-or-address>`.
2. Gather context with `python tools/ai_context.py <symbol-or-address>`.
3. Build `ninja build/GCBE7D/src/<unit>.ctx` when source context is not enough.
4. Inspect `build/GCBE7D/asm/<unit>.s` and optionally `python tools/ai_ghidra.py ... decompile <address>`.
5. Edit only the active function.
6. Build only the single object with `ninja build/GCBE7D/src/<unit>.o`.
7. Re-check with `python tools/ai_lookup_symbol.py <symbol>` and `python tools/ai_context.py <symbol>`.
8. If the function stops improving after 4 build/measure cycles, report the blocker instead of stalling.

Critical rules:

- Never use interactive `objdiff-cli diff` sessions.
- One function at a time.
- No inline asm.
- Prefer true matches with typed fields and structured control flow.
- Use Ghidra as a reverse-engineering aid, not as authoritative truth over `config/GCBE7D/symbols.txt`.
- Do not edit generated files such as `build.ninja`, `objdiff.json`, `compile_commands.json`, `config/GCBE7D/splits.generated.txt`, or `config/GCBE7D/config.generated.yml`.

Report back with:

- final match state for the target function
- blockers or follow-up ideas if the function is not done
- commands used for final verification
