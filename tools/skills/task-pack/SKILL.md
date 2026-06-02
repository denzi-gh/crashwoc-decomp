---
name: task-pack
description: Generate and run provider-neutral decompilation task packs in crashwoc-decomp. Use when an agent should turn a unit, function, or decomp.me bundle into a self-contained task pack and run it against any backend (print, copilot, codex, claude, aider, or gemini).
---

# Task Pack Workflow

A task pack is a self-contained directory describing one decompilation task. It
is backend-agnostic: the same pack works with any agent or a browser chat.

## Generate a task pack

Use `python tools/ai_task.py` with the mode that matches the target:

- `python tools/ai_task.py unit src/gamecode/crate.c`
- `python tools/ai_task.py function MoveCrate`
- `python tools/ai_task.py decompme path/to/export.zip`

The pack is written to `.ai/tasks/<timestamp>-<target>/` and contains
`task.json` (the canonical, machine-readable task), `prompt.md`, `context.md`,
`verify.sh`, `verify.ps1`, and `notes.md`. Inspect `task.json` for the resolved
unit, `allowed_edit_files`, `forbidden_edit_files`, and `verify_commands`.

## Run a task pack

- Browser ChatGPT / any agent without a local CLI:
  `python tools/ai_run.py --backend print .ai/tasks/<task>` and paste the output.
- Local GitHub Copilot:
  `python tools/ai_run.py --backend copilot .ai/tasks/<task>`.
- Best-effort local CLIs: `--backend codex|claude|aider|gemini` (each falls back
  to printing the prompt when the CLI is not on PATH).

## Rules

- Work on one function at a time; keep edits inside `allowed_edit_files`.
- Never edit the generated files listed in `forbidden_edit_files`.
- No inline asm; never start an interactive `objdiff-cli diff` session.
- Stop or mark blocked after 4 build/measure cycles without improvement, then
  record the blocker in the pack's `notes.md`.

## References

- Read [references/backends.md](references/backends.md) for backend selection and browser usage.
- Read [references/verification.md](references/verification.md) for verifying a task pack.
