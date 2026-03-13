# Match Style

These rules bias toward true matches and cleaner long-term decomp output:

- Prefer normal control flow over labels and gotos.
- Prefer typed fields, arrays, and structs over raw pointer arithmetic.
- Prefer proper structs over `extern u8[]`; split padding into real fields when the accessed layout becomes known.
- Prefer idiomatic forms such as `for` loops and `ABS` / `MIN` / `MAX` when they match the target code.
- Use `PAD_STACK(n);` only as trailing stack padding when stack size is the actual mismatch.
- Do not use inline asm for decomp work. Keep the implementation in C.
- Ignore pure register swaps while triaging, but still confirm the instruction stream and stack layout.
- Use DWARF as a clue source only. Validate names and ownership through symbols, report data, splits, and extracted asm.
