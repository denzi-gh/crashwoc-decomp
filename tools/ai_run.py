#!/usr/bin/env python3
"""Run a provider-neutral task pack against a coding backend.

Consumes a task directory created by ``tools/ai_task.py`` and dispatches it to a
backend:

    print     emit a copy/paste-friendly prompt + context block (browser chat)
    copilot   launch the GitHub Copilot CLI with the task prompt
    codex     best-effort: invoke the Codex CLI
    claude    best-effort: invoke the Claude Code CLI
    aider     best-effort: invoke aider
    gemini    best-effort: invoke the Gemini CLI

The ``print`` backend always works and is the recommended path for browser
ChatGPT, Codex Web, or any agent without a local CLI. Best-effort backends never
hard-crash when their CLI is absent: they print the prompt to paste instead.

Usage:
    python tools/ai_run.py --backend print .ai/tasks/<task>
    python tools/ai_run.py --backend copilot .ai/tasks/<task>
"""
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Callable, Optional

from ai_common import ROOT

REQUIRED_TASK_FILES = ("task.json", "prompt.md", "context.md")


class RunError(Exception):
    """User-facing error while loading or running a task pack."""


def display_path(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT.resolve()).as_posix()
    except ValueError:
        return path.as_posix()


def load_task_pack(task_dir: Path) -> tuple[dict, str, str]:
    if not task_dir.is_dir():
        raise RunError(f"Task directory not found: {task_dir}")
    for name in REQUIRED_TASK_FILES:
        if not (task_dir / name).is_file():
            raise RunError(f"Missing {name} in {display_path(task_dir)}.")
    task_json = json.loads((task_dir / "task.json").read_text(encoding="utf-8"))
    prompt_md = (task_dir / "prompt.md").read_text(encoding="utf-8")
    context_md = (task_dir / "context.md").read_text(encoding="utf-8")
    return task_json, prompt_md, context_md


def combined_prompt(prompt_md: str, context_md: str) -> str:
    return prompt_md.rstrip() + "\n\n---\n\n" + context_md.rstrip() + "\n"


# --------------------------------------------------------------------------- #
# print backend
# --------------------------------------------------------------------------- #
def backend_print(task_dir: Path, task_json: dict, prompt_md: str, context_md: str) -> int:
    lines = [
        f"# Task pack: {display_path(task_dir)}",
        "",
        "## PROMPT",
        "",
        prompt_md.rstrip(),
        "",
        "## CONTEXT",
        "",
        context_md.rstrip(),
        "",
        "## VERIFY",
        "",
        "Run `sh verify.sh` (POSIX) or `pwsh verify.ps1` (PowerShell), or directly:",
        "",
        "```sh",
        *task_json.get("verify_commands", []),
        "```",
    ]
    print("\n".join(lines))
    return 0


# --------------------------------------------------------------------------- #
# copilot backend
# --------------------------------------------------------------------------- #
def copilot_base_command(explicit_command: Optional[str]) -> list[str]:
    if explicit_command:
        if explicit_command == "gh-copilot":
            return ["gh", "copilot", "--"]
        return [explicit_command]
    if shutil.which("copilot"):
        return ["copilot"]
    if shutil.which("gh"):
        return ["gh", "copilot", "--"]
    raise RunError("Could not find `copilot` or `gh` in PATH.")


def backend_copilot(
    task_dir: Path,
    prompt_text: str,
    *,
    model: Optional[str] = None,
    copilot_command: Optional[str] = None,
) -> int:
    command = copilot_base_command(copilot_command)
    if model:
        command += ["--model", model]
    command += ["--add-dir", str(ROOT), "--no-ask-user", "-i", prompt_text]
    print(f"Launching Copilot for task pack {display_path(task_dir)} in this terminal.")
    print("Run from a VS Code integrated terminal to keep the session inside VS Code.")
    return subprocess.call(command, cwd=ROOT)


# --------------------------------------------------------------------------- #
# best-effort generic CLI backends
# --------------------------------------------------------------------------- #
def _codex_argv(prompt: str, allowed: list[str], model: Optional[str]) -> list[str]:
    argv = ["codex", "exec"]
    if model:
        argv += ["--model", model]
    return [*argv, prompt]


def _claude_argv(prompt: str, allowed: list[str], model: Optional[str]) -> list[str]:
    argv = ["claude", "-p"]
    if model:
        argv += ["--model", model]
    return [*argv, prompt]


def _aider_argv(prompt: str, allowed: list[str], model: Optional[str]) -> list[str]:
    argv = ["aider", "--yes"]
    if model:
        argv += ["--model", model]
    argv += ["--message", prompt]
    return [*argv, *allowed]


def _gemini_argv(prompt: str, allowed: list[str], model: Optional[str]) -> list[str]:
    argv = ["gemini"]
    if model:
        argv += ["--model", model]
    return [*argv, "-p", prompt]


GENERIC_BACKENDS: dict[str, Callable[[str, list[str], Optional[str]], list[str]]] = {
    "codex": _codex_argv,
    "claude": _claude_argv,
    "aider": _aider_argv,
    "gemini": _gemini_argv,
}


def backend_generic(
    name: str,
    task_dir: Path,
    task_json: dict,
    prompt_text: str,
    *,
    model: Optional[str] = None,
) -> int:
    if shutil.which(name) is None:
        print(
            f"`{name}` was not found in PATH. Showing the prompt to paste manually:\n",
            file=sys.stderr,
        )
        print(prompt_text)
        return 0
    allowed = list(task_json.get("allowed_edit_files", []))
    command = GENERIC_BACKENDS[name](prompt_text, allowed, model)
    print(
        f"Running best-effort `{name}` backend for {display_path(task_dir)}. "
        "Adjust the invocation in tools/ai_run.py if your CLI differs.",
        file=sys.stderr,
    )
    try:
        return subprocess.call(command, cwd=ROOT)
    except OSError as exc:
        print(f"Failed to launch `{name}`: {exc}", file=sys.stderr)
        print(prompt_text)
        return 1


# --------------------------------------------------------------------------- #
# CLI
# --------------------------------------------------------------------------- #
def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Run a task pack against a coding backend.")
    parser.add_argument("task_dir", help="Path to a task pack created by tools/ai_task.py")
    parser.add_argument(
        "--backend",
        default="print",
        choices=["print", "copilot", "codex", "claude", "aider", "gemini"],
        help="Backend to run (default: print).",
    )
    parser.add_argument("--model", default=None, help="Optional model name for backends that support it.")
    parser.add_argument(
        "--copilot-command",
        default=None,
        help="Copilot launcher override. Use `gh-copilot` to force the GitHub CLI wrapper.",
    )
    return parser


def main(argv: Optional[list[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    task_dir = Path(args.task_dir)
    try:
        task_json, prompt_md, context_md = load_task_pack(task_dir)
    except RunError as exc:
        print(str(exc), file=sys.stderr)
        return 1
    except json.JSONDecodeError as exc:
        print(f"Invalid task.json in {display_path(task_dir)}: {exc}", file=sys.stderr)
        return 1

    prompt_text = combined_prompt(prompt_md, context_md)

    if args.backend == "print":
        return backend_print(task_dir, task_json, prompt_md, context_md)
    if args.backend == "copilot":
        try:
            return backend_copilot(
                task_dir, prompt_text, model=args.model, copilot_command=args.copilot_command
            )
        except RunError as exc:
            print(str(exc), file=sys.stderr)
            return 1
    return backend_generic(args.backend, task_dir, task_json, prompt_text, model=args.model)


if __name__ == "__main__":
    raise SystemExit(main())
