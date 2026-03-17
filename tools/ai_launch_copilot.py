#!/usr/bin/env python3
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path

from ai_common import DEFAULT_VERSION, find_units, next_unmatched_function, unit_asm_path


ROOT = Path(__file__).resolve().parent.parent
DEFAULT_PROMPT_DIR = ROOT / ".ai" / "prompts"


def resolve_unit(query: str, version: str):
    matches = find_units(query, version=version)
    if not matches:
        raise ValueError(f"No unit matches found for '{query}'.")
    return matches[0]


def target_label(unit) -> str:
    return unit.source_path or unit.normalized_name


def build_prompt(unit, version: str) -> str:
    target = target_label(unit)
    object_path = unit.object_path or "build/GCBE7D/src/<unit>.o"
    ctx_path = unit.ctx_path or "build/GCBE7D/src/<unit>.ctx"
    asm_path = unit_asm_path(unit, version=version)
    next_function = next_unmatched_function(unit)

    lines = [
        "# Match One Unit End-to-End",
        "",
        f"Target unit or file: `{target}`",
        "",
        "Resolved paths:",
        f"- Source: `{unit.source_path or target}`",
        f"- Object: `{object_path}`",
        f"- Context: `{ctx_path}`",
        f"- Asm: `{asm_path}`",
    ]

    if next_function is not None:
        match_text = "unknown"
        if next_function.fuzzy_match_percent is not None:
            match_text = f"{next_function.fuzzy_match_percent:.2f}%"
        lines.append(
            f"- Next target: `{next_function.name}` at `0x{next_function.address:08X}` "
            f"(size `0x{next_function.size:X}`, match `{match_text}`)"
        )

    lines.extend(
        [
            "",
            "You are an autonomous decompilation agent for Crash Bandicoot: The Wrath of Cortex (GameCube).",
            "",
            "First read:",
            "",
            "- `AGENTS.md`",
            "- `.github/copilot-instructions.md`",
            "- `.github/instructions/decomp.instructions.md`",
            "",
            "Workflow:",
            "",
            f"1. Start with `python tools/ai_match_plan.py {target}`.",
            "2. Pick the first unmatched function.",
            "3. Edit only one function at a time.",
            f"4. Build only the single object with `ninja {object_path}`.",
            f"5. Build context with `ninja {ctx_path}` when source context is not enough.",
            f"6. Re-check progress with `python tools/ai_match_plan.py {target}`.",
            "7. If a function stops improving after 4 build/measure cycles, note the blocker and move to the next function.",
            "8. Repeat until every reachable function is either 100% matched or clearly blocked.",
            "",
            "Useful tools:",
            "",
            f"- `python tools/ai_lookup_unit.py {target}`",
            f"- `python tools/ai_context.py {target}`",
            "- `python tools/ai_lookup_symbol.py <name>`",
            f"- `{asm_path}`",
            "",
            "Critical rules:",
            "",
            "- Never use interactive `objdiff-cli diff` sessions.",
            "- One function at a time.",
            "- No inline asm.",
            "- Prefer true matches with typed fields and structured control flow.",
            "- Do not edit generated files such as `build.ninja`, `objdiff.json`, `compile_commands.json`, `config/GCBE7D/splits.generated.txt`, or `config/GCBE7D/config.generated.yml`.",
            "- DWARF is hints only; `config/GCBE7D/symbols.txt` is the validated source.",
            "- Do not ask questions; work autonomously and keep going.",
            "",
            "Report back with:",
            "",
            "- functions fully matched",
            "- functions skipped or blocked",
            "- commands used for final verification",
        ]
    )

    return "\n".join(lines) + "\n"


def save_prompt(prompt_text: str, unit, prompt_dir: Path) -> Path:
    prompt_dir.mkdir(parents=True, exist_ok=True)
    stem = Path(target_label(unit)).stem
    timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    prompt_path = prompt_dir / f"{timestamp}-{stem}.md"
    prompt_path.write_text(prompt_text, encoding="utf-8")
    return prompt_path


def copilot_base_command(explicit_command: str | None) -> list[str]:
    if explicit_command:
        if explicit_command == "gh-copilot":
            return ["gh", "copilot", "--"]
        return [explicit_command]

    if shutil.which("copilot"):
        return ["copilot"]
    if shutil.which("gh"):
        return ["gh", "copilot", "--"]
    raise RuntimeError("Could not find `copilot` or `gh` in PATH.")


def launch_copilot(prompt_text: str, prompt_path: Path, base_command: list[str], model: str | None) -> int:
    command = list(base_command)
    if model:
        command.extend(["--model", model])
    command.extend(["--add-dir", str(ROOT), "--no-ask-user", "-i", prompt_text])

    print(f"Prompt saved to {prompt_path}")
    print("Launching Copilot in this terminal. Run this command from a VS Code integrated terminal if you want the session to stay inside VS Code.")
    return subprocess.call(command, cwd=ROOT)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate a filled decomp prompt for a C unit and launch a Copilot CLI session in the current terminal."
    )
    parser.add_argument("query", help="Unit name or source path such as src/gamecode/crate.c")
    parser.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        help=f"Project version under config/ and build/ (default: {DEFAULT_VERSION})",
    )
    parser.add_argument(
        "--prompt-dir",
        default=str(DEFAULT_PROMPT_DIR),
        help=f"Directory used to save generated prompts (default: {DEFAULT_PROMPT_DIR})",
    )
    parser.add_argument(
        "--copilot-command",
        default=None,
        help="Copilot launcher to use. Defaults to `copilot`, with `gh copilot --` as fallback. Use `gh-copilot` to force the GitHub CLI wrapper.",
    )
    parser.add_argument(
        "--model",
        default=None,
        help="Optional Copilot CLI model name, such as gpt-5.4",
    )
    parser.add_argument(
        "--print-prompt",
        action="store_true",
        help="Print the generated prompt to stdout.",
    )
    parser.add_argument(
        "--no-launch",
        action="store_true",
        help="Only generate the prompt file; do not start Copilot.",
    )
    args = parser.parse_args()

    try:
        unit = resolve_unit(args.query, args.version)
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    prompt_text = build_prompt(unit, args.version)
    prompt_path = save_prompt(prompt_text, unit, Path(args.prompt_dir))

    if args.print_prompt:
        print(prompt_text, end="")

    if args.no_launch:
        print(f"Prompt saved to {prompt_path}")
        return 0

    try:
        base_command = copilot_base_command(args.copilot_command)
    except RuntimeError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    return launch_copilot(prompt_text, prompt_path, base_command, args.model)


if __name__ == "__main__":
    raise SystemExit(main())
