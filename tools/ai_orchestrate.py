#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
from pathlib import Path

from ai_common import (
    DEFAULT_VERSION,
    find_report_functions_by_address,
    find_report_functions_by_name,
    find_symbols_by_address,
    find_symbols_by_name,
    find_units,
    format_hex,
    format_percent,
    format_size,
    next_unmatched_function,
    parse_address,
    suggest_regression_commands,
    suggest_unit_commands,
    unit_asm_path,
)
from ai_ghidra import GhidraCliError, resolve_config, resolve_program_entry
from ai_launch_copilot import (
    DEFAULT_PROMPT_DIR,
    copilot_base_command,
    launch_copilot,
    save_prompt,
)


def quote_arg(text: str) -> str:
    return '"' + text.replace('"', '\\"') + '"'


def maybe_ghidra_context(
    project: str | None,
    ghidra_home: str | None,
    program: str | None,
) -> tuple[list[str], str | None]:
    if not project and not ghidra_home:
        return [], None

    config = resolve_config(project=project, ghidra_home=ghidra_home, program=program)

    args = [f"--project {quote_arg(str(config.project_location / (config.project_name + '.gpr')))}"]
    if config.ghidra_home != config.project_location:
        args.append(f"--ghidra-home {quote_arg(str(config.ghidra_home))}")

    try:
        program_entry = resolve_program_entry(config, program)
    except GhidraCliError:
        if program:
            args.append(f"--program {quote_arg(program)}")
            return args, program
        if config.program:
            args.append(f"--program {quote_arg(config.program)}")
            return args, config.program
        return args, None

    args.append(f"--program {quote_arg(str(program_entry['pathname']))}")
    return args, str(program_entry["pathname"])


def resolve_function_target(query: str, version: str):
    address = parse_address(query)
    if address is not None:
        function_hits = find_report_functions_by_address(address, version=version)
        symbol_hits = find_symbols_by_address(address, version=version)
    else:
        function_hits = find_report_functions_by_name(query, version=version)
        symbol_hits = find_symbols_by_name(query, version=version)

    if function_hits:
        unit, function = function_hits[0]
        symbol = symbol_hits[0] if symbol_hits else None
        return unit, function, symbol

    if symbol_hits:
        symbol = symbol_hits[0]
        unit_matches = find_units(format_hex(symbol.address), version=version)
        if unit_matches:
            return unit_matches[0], None, symbol

    raise ValueError(f"No function or symbol matches found for '{query}'.")


def build_shared_header(target_title: str) -> list[str]:
    return [
        f"# {target_title}",
        "",
        "You are an autonomous orchestration agent for Crash Bandicoot: The Wrath of Cortex (GameCube).",
        "",
        "First read:",
        "",
        "- `AGENTS.md`",
        "- `.github/copilot-instructions.md`",
        "- `.github/instructions/decomp.instructions.md`",
        "- `.github/instructions/ai-tooling.instructions.md`",
        "",
        "Orchestration rules:",
        "",
        "- You may use read-only helper scripts and read-only sub-agents for reconnaissance.",
        "- Keep exactly one active code-writing flow at a time.",
        "- For decomp edits, keep one active function selected at all times.",
        "- Never use interactive `objdiff-cli diff` sessions.",
        "- No inline asm.",
        "- If a function stalls for 4 build/measure cycles, note the blocker and move on.",
        "",
    ]


def build_unit_prompt(
    unit,
    version: str,
    ghidra_args: list[str],
    ghidra_program: str | None,
) -> str:
    target = unit.source_path or unit.normalized_name
    object_path = unit.object_path or "build/GCBE7D/src/<unit>.o"
    ctx_path = unit.ctx_path or "build/GCBE7D/src/<unit>.ctx"
    asm_path = unit_asm_path(unit, version=version)
    next_function = next_unmatched_function(unit)
    ghidra_prefix = "python tools/ai_ghidra.py"
    if ghidra_args:
        ghidra_prefix += " " + " ".join(ghidra_args)

    lines = build_shared_header("Orchestrate One Unit")
    lines.extend(
        [
            f"Target unit or file: `{target}`",
            "",
            "Resolved paths:",
            f"- Source: `{unit.source_path or target}`",
            f"- Object: `{object_path}`",
            f"- Context: `{ctx_path}`",
            f"- Asm: `{asm_path}`",
        ]
    )

    if next_function is not None:
        lines.append(
            f"- Next target: `{next_function.name}` at `{format_hex(next_function.address)}` "
            f"(size `{format_size(next_function.size)}`, match `{format_percent(next_function.fuzzy_match_percent)}`)"
        )
    if ghidra_program:
        lines.extend(["- Ghidra program: `{}`".format(ghidra_program), ""])
    else:
        lines.append("")

    lines.extend(
        [
            "Workflow:",
            "",
            f"1. Start with `python tools/ai_match_plan.py {target}`.",
            "2. Choose the next active function and keep it explicit in your notes/output.",
            f"3. Use `python tools/ai_context.py {target}` and `python tools/ai_lookup_unit.py {target}` to refresh context.",
        ]
    )
    if ghidra_args:
        lines.append(
            f"4. Use Ghidra when source context is thin, for example `{ghidra_prefix} find-function {quote_arg(next_function.name if next_function else Path(target).stem)}`."
        )
        lines.append(
            f"5. Build `ninja {ctx_path}` when you need flattened source context, then inspect `{asm_path}` or `{ghidra_prefix} decompile {format_hex(next_function.address) if next_function else '<addr>'}`."
        )
        lines.append(f"6. Edit only the active function, then build `ninja {object_path}`.")
        lines.append(f"7. Re-run `python tools/ai_match_plan.py {target}` and targeted lookup helpers.")
        lines.append("8. Repeat until every reachable function is matched or clearly blocked.")
    else:
        lines.append(f"4. Build `ninja {ctx_path}` when you need flattened source context, then inspect `{asm_path}`.")
        lines.append(f"5. Edit only the active function, then build `ninja {object_path}`.")
        lines.append(f"6. Re-run `python tools/ai_match_plan.py {target}` and targeted lookup helpers.")
        lines.append("7. Repeat until every reachable function is matched or clearly blocked.")

    lines.extend(
        [
            "",
            "Useful commands:",
            "",
            f"- `python tools/ai_lookup_unit.py {target}`",
            f"- `python tools/ai_context.py {target}`",
            f"- `python tools/ai_match_plan.py {target}`",
            f"- `ninja {object_path}`",
            f"- `ninja {ctx_path}`",
            f"- `{asm_path}`",
        ]
    )
    if ghidra_args:
        lines.extend(
            [
                f"- `{ghidra_prefix} programs`",
                f"- `{ghidra_prefix} find-function {quote_arg(next_function.name if next_function else Path(target).stem)}`",
            ]
        )
    lines.extend(
        [
            *[f"- `{command}`" for command in suggest_regression_commands()],
            "",
            "Report back with:",
            "",
            "- active function reached or final next target",
            "- functions fully matched",
            "- functions skipped or blocked",
            "- commands used for final verification",
        ]
    )
    return "\n".join(lines) + "\n"


def build_function_prompt(
    query: str,
    unit,
    function,
    symbol,
    version: str,
    ghidra_args: list[str],
    ghidra_program: str | None,
) -> str:
    target_name = function.name if function is not None else symbol.name
    address = (
        function.address
        if function is not None
        else symbol.address
    )
    size = function.size if function is not None else symbol.size
    match_percent = function.fuzzy_match_percent if function is not None else None
    target = unit.source_path or unit.normalized_name
    object_path = unit.object_path or "build/GCBE7D/src/<unit>.o"
    ctx_path = unit.ctx_path or "build/GCBE7D/src/<unit>.ctx"
    asm_path = unit_asm_path(unit, version=version)
    ghidra_prefix = "python tools/ai_ghidra.py"
    if ghidra_args:
        ghidra_prefix += " " + " ".join(ghidra_args)

    lines = build_shared_header("Match One Function")
    lines.extend(
        [
            f"Target query: `{query}`",
            f"Resolved function: `{target_name}`",
            f"Address: `{format_hex(address)}`",
            f"Size: `{format_size(size)}`",
            f"Match: `{format_percent(match_percent)}`",
            f"Unit: `{unit.normalized_name}`",
            f"Source: `{target}`",
            f"Object: `{object_path}`",
            f"Context: `{ctx_path}`",
            f"Asm: `{asm_path}`",
        ]
    )
    if ghidra_program:
        lines.append(f"Ghidra program: `{ghidra_program}`")
    lines.extend(
        [
            "",
            "Workflow:",
            "",
            f"1. Refresh context with `python tools/ai_context.py {target_name}` and `python tools/ai_lookup_symbol.py {target_name}`.",
            f"2. Build `ninja {ctx_path}` if the local source view is not enough.",
            f"3. Inspect `{asm_path}` for exact control flow, constants, and nearby labels.",
        ]
    )
    if ghidra_args:
        lines.append(
            f"4. Use Ghidra for the original-side view, for example `{ghidra_prefix} decompile {format_hex(address)}` and `{ghidra_prefix} disasm {format_hex(address)} -n 30`."
        )
        lines.append(f"5. Edit only `{target_name}`, then build `ninja {object_path}`.")
        lines.append(f"6. Re-check with `python tools/ai_lookup_symbol.py {target_name}` and `python tools/ai_context.py {target_name}`.")
    else:
        lines.append(f"4. Edit only `{target_name}`, then build `ninja {object_path}`.")
        lines.append(f"5. Re-check with `python tools/ai_lookup_symbol.py {target_name}` and `python tools/ai_context.py {target_name}`.")

    lines.extend(
        [
            "",
            "Useful commands:",
            "",
            f"- `python tools/ai_lookup_symbol.py {target_name}`",
            f"- `python tools/ai_context.py {target_name}`",
            f"- `python tools/ai_lookup_unit.py {unit.normalized_name}`",
            f"- `ninja {ctx_path}`",
            f"- `ninja {object_path}`",
            f"- `{asm_path}`",
        ]
    )
    if ghidra_args:
        lines.extend(
            [
                f"- `{ghidra_prefix} find-function {quote_arg(target_name)}`",
                f"- `{ghidra_prefix} type-get {quote_arg(Path(target).stem)}`",
            ]
        )
    lines.extend(
        [
            *[f"- `{command}`" for command in suggest_regression_commands()],
            "",
            "Report back with:",
            "",
            "- final match state for the target function",
            "- exact blocker if it did not improve",
            "- commands used for final verification",
        ]
    )
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate a Copilot orchestration prompt for one unit or one function."
    )
    target_group = parser.add_mutually_exclusive_group(required=True)
    target_group.add_argument("--unit", help="Unit name or source path such as src/gamecode/crate.c")
    target_group.add_argument("--function", help="Function name or address such as MoveCrate or 0x800032A0")
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
        "--project",
        default=None,
        help="Path to a Ghidra project directory or .gpr file.",
    )
    parser.add_argument(
        "--ghidra-home",
        default=None,
        help="Path to the Ghidra install root when it cannot be inferred from --project.",
    )
    parser.add_argument(
        "--program",
        default=None,
        help="Program name or project pathname inside the Ghidra project.",
    )
    parser.add_argument(
        "--copilot-command",
        default=None,
        help="Copilot launcher to use. Defaults to `copilot`, with `gh copilot --` as fallback.",
    )
    parser.add_argument(
        "--model",
        default=None,
        help="Optional Copilot CLI model name, such as gpt-5.4.",
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

    ghidra_args: list[str] = []
    ghidra_program: str | None = None
    # Use --project / --ghidra-home if given, otherwise fall back to env vars
    # so Ghidra context is always included when CRASHWOC_GHIDRA_PROJECT is set.
    ghidra_project = args.project or os.environ.get("CRASHWOC_GHIDRA_PROJECT") or os.environ.get("GHIDRA_PROJECT_PATH")
    ghidra_home = args.ghidra_home or os.environ.get("CRASHWOC_GHIDRA_HOME") or os.environ.get("GHIDRA_HOME")
    if ghidra_project or ghidra_home:
        try:
            ghidra_args, ghidra_program = maybe_ghidra_context(
                project=ghidra_project,
                ghidra_home=ghidra_home,
                program=args.program,
            )
        except GhidraCliError as exc:
            print(str(exc), file=sys.stderr)
            return 1

    try:
        if args.unit:
            matches = find_units(args.unit, version=args.version)
            if not matches:
                raise ValueError(f"No unit matches found for '{args.unit}'.")
            unit = matches[0]
            prompt_text = build_unit_prompt(
                unit,
                version=args.version,
                ghidra_args=ghidra_args,
                ghidra_program=ghidra_program,
            )
            prompt_path = save_prompt(prompt_text, unit, Path(args.prompt_dir))
        else:
            unit, function, symbol = resolve_function_target(args.function, args.version)
            prompt_text = build_function_prompt(
                args.function,
                unit,
                function,
                symbol,
                version=args.version,
                ghidra_args=ghidra_args,
                ghidra_program=ghidra_program,
            )
            prompt_path = save_prompt(prompt_text, unit, Path(args.prompt_dir))
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        return 1

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
