# Verification

## Default Sequence

1. Resolve the symbol or unit with the lookup helpers.
2. If the task starts from a file or unit, run `python tools/ai_match_plan.py <unit-or-path>` to refresh the remaining-function backlog.
3. Build the unit context target if you need flattened source context.
4. Build the single object target after editing.
5. Re-run `python tools/ai_lookup_symbol.py <symbol>`, `python tools/ai_context.py <symbol>`, or `python tools/ai_match_plan.py <unit>` to confirm the target still resolves and to inspect current match data.
6. Run `ninja changes` if the edit can affect matching or progress. If `build/GCBE7D/baseline.json` is missing, run `ninja baseline` first.

## Useful Commands

```sh
python tools/ai_context.py <symbol-or-unit>
python tools/ai_match_plan.py <unit-or-path>
ninja build/GCBE7D/src/<unit>.ctx
ninja build/GCBE7D/src/<unit>.o
python tools/ai_lookup_symbol.py <symbol>
ninja baseline
ninja changes
python tools/changes_fmt.py build/GCBE7D/report_changes.json
```

Use the file-producing report commands above for automation. Avoid interactive
`objdiff-cli diff` sessions because they stay open and keep refreshing.

## Exit Criteria

- The intended source file builds cleanly.
- The lookup helpers still resolve the target cleanly.
- The symbol or unit reports the expected match state after the edit.
- No unexpected regressions appear in `ninja changes` when that check is relevant.
- A freshly created baseline is treated only as a prerequisite for `ninja changes`, not as a before/after regression reference for the same edit.
