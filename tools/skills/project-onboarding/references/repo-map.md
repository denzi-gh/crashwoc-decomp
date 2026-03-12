# Repo Map

## Primary Project Files

- [`../../../AGENTS.md`](../../../AGENTS.md): repo-level AI workflow summary
- [`../../../README.md`](../../../README.md): human-facing project overview
- [`../../../configure.py`](../../../configure.py): build generation and object list source
- [`../../../config/GCBE7D/config.yml`](../../../config/GCBE7D/config.yml): checked-in dtk config
- [`../../../config/GCBE7D/splits.txt`](../../../config/GCBE7D/splits.txt): checked-in split ownership
- [`../../../config/GCBE7D/symbols.txt`](../../../config/GCBE7D/symbols.txt): validated symbols
- [`../../../src/dump_alphaNGCport_DWARF.txt`](../../../src/dump_alphaNGCport_DWARF.txt): fallback debug dump

## Generated Build Files

- `../../../build.ninja`: single-object and context targets
- `../../../objdiff.json`: objdiff config
- `../../../compile_commands.json`: source-to-object mapping for generated targets
- `../../../build/GCBE7D/report.json`: unit and function progress data after a build

## Helper Scripts

- `../../../tools/ai_lookup_symbol.py`
- `../../../tools/ai_lookup_unit.py`
- `../../../tools/ai_context.py`
- `../../../tools/ai_decompme_zip.py`
- `../../../tools/triage_build.py`
- `../../../tools/add_split_from_backup.py`
- `../../../tools/generate_seed_splits.py`
- `../../../tools/extract_symbol_drafts.py`
