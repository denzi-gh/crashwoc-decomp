# Verification

## Default Sequence

1. Resolve the symbol or unit with the lookup helpers.
2. Build the unit context target if you need flattened source context.
3. Build the single object target after editing.
4. Run `ninja changes` if the edit can affect matching or progress.

## Useful Commands

```sh
python tools/ai_context.py <symbol-or-unit>
ninja build/GCBE7D/src/<unit>.ctx
ninja build/GCBE7D/src/<unit>.o
ninja changes
```

## Exit Criteria

- The intended source file builds cleanly.
- The lookup helpers still resolve the target cleanly.
- No unexpected regressions appear in `ninja changes` when that check is relevant.
