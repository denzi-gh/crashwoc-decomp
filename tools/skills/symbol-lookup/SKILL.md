---
name: symbol-lookup
description: Lookup symbols, addresses, units, and match metadata in crashwoc-decomp. Use when Codex needs to map a function or global to config/GCBE7D/symbols.txt, split ownership, build/GCBE7D/report.json, or fallback DWARF lines.
---

# Symbol Lookup

## Primary Commands

Start with the repo-local helpers:

- `python tools/ai_lookup_symbol.py <name-or-address>`
- `python tools/ai_context.py <name-or-address>`
- `python tools/ai_lookup_unit.py <unit-or-path>` when the query is really a source file or unit

Examples:

- `python tools/ai_lookup_symbol.py FindAIType`
- `python tools/ai_lookup_symbol.py 0x800032A0`
- `python tools/ai_context.py FindAIType`

## Source Priority

Use the lookup sources in this order:

1. `config/GCBE7D/symbols.txt` for validated names and addresses
2. `build/GCBE7D/report.json` for owning source paths, progress categories, and fuzzy-match data
3. `build/GCBE7D/asm/<unit>.s` when you need exact function control flow, rodata strings, or float constants from the extracted retail assembly
4. `config/GCBE7D/splits.txt` to resolve the owning unit from an address range
5. `src/dump_alphaNGCport_DWARF.txt` for fallback hints only

Do not promote DWARF hints into validated facts without checking the rest of the repo state.

## Working Pattern

- If you have a symbol or address, use `ai_lookup_symbol.py` first.
- If you need the source path, object target, context target, and nearby functions, use `ai_context.py`.
- If helper output and source context are not enough, inspect `build/GCBE7D/asm/<unit>.s` for the exact body and nearby rodata labels before falling back to DWARF.
- If the lookup returns multiple unit candidates because of duplicate basenames, switch to the normalized unit name printed by the helper, for example `ai_1.c`.

## References

- Read [references/data-sources.md](references/data-sources.md) for the important files, special cases, and examples.
