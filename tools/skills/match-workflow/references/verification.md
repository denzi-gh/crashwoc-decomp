# Verification

## Default Sequence

1. Resolve the symbol or unit with the lookup helpers.
2. Build the unit context target if you need flattened source context.
3. Build the single object target after editing.
4. Re-run `python tools/ai_lookup_symbol.py <symbol>` or `python tools/ai_context.py <symbol>` to confirm the target still resolves and to inspect current match data.
5. Run `ninja changes` if the edit can affect matching or progress. If `build/GCBE7D/baseline.json` is missing, run `ninja baseline` first.

## Useful Commands

```sh
python tools/ai_context.py <symbol-or-unit>
ninja build/GCBE7D/src/<unit>.ctx
ninja build/GCBE7D/src/<unit>.o
python tools/ai_lookup_symbol.py <symbol>
ninja baseline
ninja changes
```

## Exit Criteria

- The intended source file builds cleanly.
- The lookup helpers still resolve the target cleanly.
- The symbol or unit reports the expected match state after the edit.
- No unexpected regressions appear in `ninja changes` when that check is relevant.
- A freshly created baseline is treated only as a prerequisite for `ninja changes`, not as a before/after regression reference for the same edit.
