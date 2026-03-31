---
name: agent-orchestration
description: Orchestrate one Crash WOC unit or one function with Copilot using the repo-local helper scripts, prompt entry points, and a one-writer-at-a-time workflow.
---

# Agent Orchestration

## Purpose

Use this skill when the task is larger than a one-off lookup but should still stay tightly scoped to one file or one function.

## Entry Points

- `python tools/ai_orchestrate.py --unit <unit-or-path>`
- `python tools/ai_orchestrate.py --function <symbol-or-address>`
- `.github/prompts/orchestrate-unit.prompt.md`
- `.github/prompts/match-function.prompt.md`
- `.github/prompts/reverse-engineer.prompt.md`

## Operating Rules

- Use read-only helper scripts and read-only exploration freely.
- Keep exactly one active code-writing flow at a time.
- Keep one active function selected at all times for decomp edits.
- Prefer `ai_lookup_symbol.py`, `ai_lookup_unit.py`, `ai_context.py`, `ai_match_plan.py`, and `ai_ghidra.py` over ad hoc repo-wide searching.
- Never use interactive `objdiff-cli diff` sessions in automated loops.

## Suggested Unit Workflow

1. Run `python tools/ai_orchestrate.py --unit <unit-or-path> --no-launch --print-prompt`.
2. Review the chosen next function and the generated helper commands.
3. Launch Copilot with the same command without `--no-launch` when ready.
4. Re-run `python tools/ai_match_plan.py <unit-or-path>` after each measurable change.
5. If a function stalls for 4 build/measure cycles, note the blocker and move on.

## Suggested Function Workflow

1. Run `python tools/ai_orchestrate.py --function <symbol-or-address> --no-launch --print-prompt`.
2. Refresh lookup and context with `ai_lookup_symbol.py` and `ai_context.py`.
3. Use `ai_ghidra.py` if the original-side context is still unclear.
4. Edit only the target function, then build only the owning object.

## References

- Read [../match-workflow/SKILL.md](../match-workflow/SKILL.md) for the matching loop.
- Read [../ghidra-workflow/SKILL.md](../ghidra-workflow/SKILL.md) for Ghidra CLI usage.
