# Match One Unit End-to-End

Use this prompt directly in GitHub Copilot when you want it to take one file or unit and keep pushing it forward.
If you want a prefilled prompt plus an interactive terminal Copilot session, run `python tools/ai_launch_copilot.py <target-file-or-unit>` from a VS Code integrated terminal.

Target unit or file: `<replace with src/... path or unit name>`

You are an autonomous decompilation agent for Crash Bandicoot: The Wrath of Cortex (GameCube).

First read:

- `AGENTS.md`
- `.github/copilot-instructions.md`
- `.github/instructions/decomp.instructions.md`

Workflow:

1. Start with `python tools/ai_match_plan.py <target-file-or-unit>`.
2. Pick the first unmatched function.
3. Edit only one function at a time.
4. Build only the single object with `ninja build/GCBE7D/src/<unit>.o`.
5. Re-check progress with `python tools/ai_match_plan.py <target-file-or-unit>`.
6. If a function stops improving after 4 build/measure cycles, note the blocker and move to the next function.
7. Repeat until every reachable function is either 100% matched or clearly blocked.

Useful tools:

- `python tools/ai_lookup_symbol.py <name>`
- `python tools/ai_lookup_unit.py <name-or-path>`
- `python tools/ai_context.py <name-or-unit>`
- `build/GCBE7D/asm/<unit>.s`
- `ninja build/GCBE7D/src/<unit>.ctx`

Critical rules:

- Never use interactive `objdiff-cli diff` sessions.
- One function at a time.
- No inline asm.
- Prefer true matches with typed fields and structured control flow.
- Do not edit generated files such as `build.ninja`, `objdiff.json`, `compile_commands.json`, `config/GCBE7D/splits.generated.txt`, or `config/GCBE7D/config.generated.yml`.
- DWARF is hints only; `config/GCBE7D/symbols.txt` is the validated source.
- Do not ask questions; work autonomously and keep going.

Report back with:

- functions fully matched
- functions skipped or blocked
- commands used for final verification
