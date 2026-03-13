# Match One Unit End-to-End

Take the source file or unit below and keep working until every reachable function is either 100% matched or clearly blocked.

Target unit or file: `<replace with src/... path or unit name>`

Read `AGENTS.md` for the full workflow, helper scripts, and repo map. Follow the Autonomous Matching Loop described there.

Critical rules:

- Never use interactive `objdiff-cli diff` sessions — use `objdiff-cli report generate` or `ninja changes`.
- One function at a time; rebuild single object (`ninja build/GCBE7D/src/<unit>.o`) after each edit.
- After 4 no-progress cycles, note the blocker and move to the next function.
- No inline asm. Prefer true matches with typed fields and structured control flow.
- DWARF is hints only; `config/GCBE7D/symbols.txt` is the validated source.

Report back with:

- functions fully matched
- functions skipped or blocked
- commands used for final verification
