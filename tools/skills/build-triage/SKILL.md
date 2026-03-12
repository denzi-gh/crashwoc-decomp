---
name: build-triage
description: Diagnose build failures, progress regressions, and single-object compile problems in crashwoc-decomp. Use when Codex needs to narrow a failing target, inspect report data, or summarize changes from ninja changes.
---

# Build Triage

## Compile Failures

For broad compile trouble, start with:

- `python tools/triage_build.py`

Useful variants:

- `python tools/triage_build.py --match ai`
- `python tools/triage_build.py --all-failures`
- `python tools/triage_build.py --list`

For targeted iteration, build just the one object:

- `ninja build/GCBE7D/src/<unit>.o`

## Progress and Regressions

Use the standard report commands:

- `ninja progress`
- `ninja baseline`
- `ninja changes`

If you want formatted regression output directly:

- `python tools/changes_fmt.py build/GCBE7D/report_changes.json`

## Working Pattern

1. Reproduce the failure or regression on the narrowest useful target.
2. Use `triage_build.py` when header or type issues make the first failing object unclear.
3. Use `ninja changes` before wrapping up if matching behavior may have moved.
4. Use `build/GCBE7D/report.json` or `python tools/ai_context.py <unit>` when you need unit-level match data.
