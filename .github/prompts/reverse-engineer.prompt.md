# Reverse Engineer With Ghidra

Use this prompt when you want Copilot to improve understanding before or alongside matching work.

Target symbol, address, or unit: `<replace with symbol, 0xADDRESS, or src/... path>`

You are a reverse-engineering assistant for Crash Bandicoot: The Wrath of Cortex (GameCube).

First read:

- `AGENTS.md`
- `.github/copilot-instructions.md`
- `.github/instructions/decomp.instructions.md`

Workflow:

1. Resolve the target with `python tools/ai_lookup_symbol.py ...`, `python tools/ai_lookup_unit.py ...`, or `python tools/ai_context.py ...`.
2. Use `python tools/ai_ghidra.py programs` to confirm the configured program if needed.
3. Use `python tools/ai_ghidra.py find-function ...`, `decompile ...`, `disasm ...`, and `type-get ...` to gather Ghidra-side evidence.
4. Cross-check Ghidra output against:
   - `config/GCBE7D/symbols.txt`
   - `config/GCBE7D/splits.txt`
   - `build/GCBE7D/report.json`
   - `build/GCBE7D/asm/<unit>.s`
5. Write down concrete findings that can unblock matching work: likely field types, control-flow shape, constant meaning, or nearby helper relationships.

Critical rules:

- Ghidra is a clue source, not automatically trusted ground truth.
- Prefer validated repo data over speculative renaming.
- Do not start uncontrolled edit loops while still in reconnaissance mode.

Report back with:

- what Ghidra clarified
- what still needs validation from source or asm
- the best next matching step
