---
name: split-maintenance
description: Maintain config/GCBE7D/splits.txt and its generated companions. Use when Codex needs to add or replace unit ranges, import split data from the backup file, seed split ownership, or reason about normalized unit names like ai_1.c and GBA_1.c.
---

# Split Maintenance

## Core Rules

- Edit `config/GCBE7D/splits.txt`, not `config/GCBE7D/splits.generated.txt`.
- Rerun `python configure.py --version GCBE7D --toolchain prodg35` after changing splits or config files.
- Treat `config/GCBE7D/splits.elf_import_backup.txt` as a source of candidate ranges, not as something to copy wholesale without review.

## Preferred Tools

For safe import from the backup file:

- `python tools/add_split_from_backup.py --unit <unit>`
- Start with the default `.text,.rodata` import unless the task clearly needs more sections.

For seed generation from map-style comments:

- `python tools/generate_seed_splits.py --version GCBE7D`

For lookup while editing:

- `python tools/ai_lookup_unit.py <unit-or-address>`
- `python tools/ai_context.py <unit-or-address>`

## Working Pattern

1. Resolve the target unit and inspect its current ranges.
2. Import or edit only the sections you actually need.
3. Rerun configure.
4. Build the affected object or run a full `ninja` if the change is broad.

## References

- Read [references/splits-format.md](references/splits-format.md) for the file format, generated outputs, and normalization rules.
