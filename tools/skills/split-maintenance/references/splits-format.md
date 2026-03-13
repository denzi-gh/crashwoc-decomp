# Splits Format

## Source of Truth

- Edit [`../../../../config/GCBE7D/splits.txt`](../../../../config/GCBE7D/splits.txt).
- Do not hand-edit `../../../config/GCBE7D/splits.generated.txt`.

`configure.py` runs `tools/normalize_splits.py` to keep the generated files tool-safe.

## Section Blocks

Each unit block starts with a unit header such as `ai.c:` and then one or more tab-indented ranges:

```text
ai.c:
    .text       start:0x800032A0 end:0x80008F3C
    .rodata     start:0x80104960 end:0x80104CD0
```

## Normalized Names

Some units use normalized names in generated artifacts and build targets:

- `ai_1.c`
- `GBA_1.c`

Use `python tools/ai_lookup_unit.py <query>` when you are not sure which normalized name the build uses.

## Safe Import Workflow

Use:

```sh
python tools/add_split_from_backup.py --unit <unit>
```

The default import is intentionally conservative and starts with `.text,.rodata`.
