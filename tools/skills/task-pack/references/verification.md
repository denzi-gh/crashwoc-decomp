# Verifying a Task Pack

Each task pack carries the canonical verify commands in `task.json`
(`verify_commands`) and as runnable scripts:

- POSIX / git-bash / WSL: `sh .ai/tasks/<task>/verify.sh`
- Windows PowerShell: `pwsh .ai/tasks/<task>/verify.ps1`

The default commands build the single object, refresh the match plan, and run a
regression check:

```sh
ninja build/GCBE7D/src/<unit>.o
python tools/ai_match_plan.py <source>
ninja changes
python tools/changes_fmt.py build/GCBE7D/report_changes.json
```

Notes:

- `verify.sh` and `verify.ps1` are generated from the same `verify_commands`, so
  they stay in sync; `task.json` is the source of truth.
- If `build/GCBE7D/baseline.json` is missing, run `ninja baseline` before
  `ninja changes`.
- Never use interactive `objdiff-cli diff`; use `ninja changes` plus
  `changes_fmt.py` for non-interactive regression output.
