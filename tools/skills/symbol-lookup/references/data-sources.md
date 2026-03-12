# Symbol Lookup Data Sources

## Validated Data

- [`../../../config/GCBE7D/symbols.txt`](../../../config/GCBE7D/symbols.txt) stores validated names, addresses, sizes, scopes, and basic symbol types.
- [`../../../config/GCBE7D/splits.txt`](../../../config/GCBE7D/splits.txt) maps address ranges back to units and sections.

## Generated Data

- `../../../build/GCBE7D/report.json` links source paths, progress categories, and per-function fuzzy match values.
- `../../../build.ninja` exposes exact single-object and context targets.
- `../../../build/GCBE7D/asm/<unit>.s` exposes the extracted retail assembly, nearby rodata labels, and in-file constants for a resolved unit.

## Fallback Data

- [`../../../src/dump_alphaNGCport_DWARF.txt`](../../../src/dump_alphaNGCport_DWARF.txt) is the large fallback dump for hints when validated repo data is missing.

## Special Cases

- Duplicate basenames are normalized in generated targets, for example `ai.c` versus `ai_1.c`.
- `GBA.c` becomes `GBA_1.c` in normalized unit naming.
- Prefer the extracted unit asm before large manual DWARF searching when you already know the owning unit.
- Prefer validated symbols and report metadata before trusting a DWARF line in isolation.
