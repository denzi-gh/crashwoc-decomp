#!/usr/bin/env python3
"""Compatibility wrapper: build a unit task pack and (optionally) launch Copilot.

This used to be the primary AI entry point. It now delegates to the
provider-neutral task-pack workflow so all backends share one task description:

    tools/ai_task.py   builds the task pack (.ai/tasks/<task>)
    tools/ai_run.py    runs the copilot backend against it

Existing commands keep working:
    python tools/ai_launch_copilot.py src/gamecode/crate.c
    python tools/ai_launch_copilot.py src/gamecode/crate.c --no-launch --print-prompt

For new work prefer the provider-neutral workflow:
    python tools/ai_task.py unit src/gamecode/crate.c
    python tools/ai_run.py --backend print .ai/tasks/<task>
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

import ai_run
import ai_task
from ai_common import DEFAULT_VERSION


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Compatibility wrapper that builds a unit task pack and launches the Copilot CLI.",
    )
    parser.add_argument("query", help="Unit name or source path such as src/gamecode/crate.c")
    parser.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        help=f"Project version under config/ and build/ (default: {DEFAULT_VERSION})",
    )
    parser.add_argument(
        "--out-dir",
        default=str(ai_task.DEFAULT_OUT_DIR),
        help="Directory for generated task packs (default: .ai/tasks).",
    )
    parser.add_argument(
        "--prompt-dir",
        default=None,
        help="Deprecated alias for --out-dir (kept for backwards compatibility).",
    )
    parser.add_argument(
        "--copilot-command",
        default=None,
        help="Copilot launcher override. Use `gh-copilot` to force the GitHub CLI wrapper.",
    )
    parser.add_argument("--model", default=None, help="Optional Copilot CLI model name, such as gpt-5.4.")
    parser.add_argument("--print-prompt", action="store_true", help="Print the generated prompt to stdout.")
    parser.add_argument(
        "--no-launch",
        action="store_true",
        help="Only generate the task pack; do not start Copilot.",
    )
    parser.add_argument("--force", action="store_true", help="Overwrite an existing task directory.")
    args = parser.parse_args(argv)

    out_dir = args.prompt_dir or args.out_dir
    if args.prompt_dir:
        print(
            "Note: --prompt-dir is deprecated; a full task pack is written under it instead of a bare prompt.",
            file=sys.stderr,
        )

    try:
        plan = ai_task.build_unit_task(args.query, args.version)
        task_dir = ai_task.write_task_pack(plan, Path(out_dir), args.force)
    except ai_task.TaskError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    shown = ai_task.display_path(task_dir)
    if args.print_prompt:
        print((task_dir / "prompt.md").read_text(encoding="utf-8"), end="")

    if args.no_launch:
        print(f"Task pack created: {shown}", file=sys.stderr)
        return 0

    try:
        _, prompt_md, context_md = ai_run.load_task_pack(task_dir)
        prompt_text = ai_run.combined_prompt(prompt_md, context_md)
        return ai_run.backend_copilot(
            task_dir, prompt_text, model=args.model, copilot_command=args.copilot_command
        )
    except ai_run.RunError as exc:
        print(str(exc), file=sys.stderr)
        print(f"Task pack is ready at {shown}; run `python tools/ai_run.py --backend print {shown}`.", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
