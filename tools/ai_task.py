#!/usr/bin/env python3
"""Generate provider-neutral decompilation task packs.

A *task pack* is a self-contained directory under
``.ai/tasks/<timestamp>-<target>/`` describing a single decompilation task. It
contains:

    task.json    canonical, machine-readable task description
    prompt.md    agent-facing instructions (provider-neutral)
    context.md   concise repo context snapshot
    verify.sh    one-shot verification commands
    notes.md     scratch space for the working agent

Task packs are deliberately backend-agnostic. They can be consumed by
``tools/ai_run.py`` (print/copilot/codex/claude/aider/gemini), pasted into a
browser chat session, or read directly by any agent.

Usage:
    python tools/ai_task.py unit src/gamecode/crate.c
    python tools/ai_task.py function MoveCrate
    python tools/ai_task.py decompme path/to/export.zip
"""
from __future__ import annotations

import argparse
import json
import re
import sys
import zipfile
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Any, Optional

import ai_decompme_zip
from ai_common import (
    DEFAULT_VERSION,
    ROOT,
    UnitInfo,
    find_units,
    format_hex,
    format_percent,
    format_size,
    load_repo_index,
    nearby_functions,
    next_unmatched_function,
    ranked_unit_functions,
    rel_posix,
    suggest_regression_commands,
    suggest_unit_commands,
    unit_asm_path,
    unit_match_counts,
)
from ai_context import resolve_context

DEFAULT_OUT_DIR = ROOT / ".ai" / "tasks"
TASK_FILES = ("task.json", "prompt.md", "context.md", "verify.sh", "notes.md")
SCHEMA_VERSION = 1

STOP_RULES = {
    "one_function_at_a_time": True,
    "max_cycles_without_improvement": 4,
    "no_inline_asm": True,
    "no_interactive_objdiff": True,
}

NOTES_TEMPLATE = """# Agent Notes

## Active target

## Attempts

## Blockers

## Verification

## Final result
"""


class TaskError(Exception):
    """User-facing error while resolving or building a task pack."""


@dataclass
class TaskPlan:
    kind: str
    slug: str
    target_label: str
    task_json: dict[str, Any]
    prompt_md: str
    context_md: str


# --------------------------------------------------------------------------- #
# Path helpers
# --------------------------------------------------------------------------- #
def display_path(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT.resolve()).as_posix()
    except ValueError:
        return path.as_posix()


def slugify(text: str) -> str:
    name = Path(str(text).replace("\\", "/")).name
    base = Path(name).stem or name
    safe = re.sub(r"[^A-Za-z0-9_.-]+", "-", base).strip("-._")
    return safe or "task"


def source_no_ext(source_path: str) -> str:
    suffix = Path(source_path).suffix
    if suffix:
        return source_path[: -len(suffix)]
    return source_path


def derive_object_path(unit: Optional[UnitInfo], version: str) -> Optional[str]:
    if unit is None:
        return None
    if unit.object_path:
        return unit.object_path
    if unit.source_path:
        return f"build/{version}/{source_no_ext(unit.source_path)}.o"
    return None


def derive_ctx_path(unit: Optional[UnitInfo], version: str) -> Optional[str]:
    if unit is None:
        return None
    if unit.ctx_path:
        return unit.ctx_path
    if unit.source_path:
        return f"build/{version}/{source_no_ext(unit.source_path)}.ctx"
    return None


def derive_asm_path(unit: Optional[UnitInfo], version: str) -> Optional[str]:
    if unit is None:
        return None
    if unit.source_path or unit.normalized_name:
        return unit_asm_path(unit, version=version)
    return None


# --------------------------------------------------------------------------- #
# Shared task.json builders
# --------------------------------------------------------------------------- #
def forbidden_edit_files(version: str) -> list[str]:
    return [
        "build.ninja",
        "objdiff.json",
        "compile_commands.json",
        f"config/{version}/splits.generated.txt",
        f"config/{version}/config.generated.yml",
    ]


def unit_block(unit: Optional[UnitInfo], version: str) -> Optional[dict[str, Any]]:
    if unit is None:
        return None
    return {
        "name": unit.raw_name,
        "normalized_name": unit.normalized_name,
        "source_path": unit.source_path,
        "object_path": derive_object_path(unit, version),
        "ctx_path": derive_ctx_path(unit, version),
        "asm_path": derive_asm_path(unit, version),
    }


def verify_commands(unit: Optional[UnitInfo], version: str) -> list[str]:
    commands: list[str] = []
    if unit is not None:
        object_path = derive_object_path(unit, version)
        target = unit.source_path or unit.normalized_name
        if object_path:
            commands.append(f"ninja {object_path}")
        commands.append(f"python tools/ai_match_plan.py {target}")
    commands.append("ninja changes")
    commands.append(f"python tools/changes_fmt.py build/{version}/report_changes.json")
    return commands


def allowed_edit_files(unit: Optional[UnitInfo]) -> list[str]:
    if unit is not None and unit.source_path:
        return [unit.source_path]
    return []


def dedupe(items: list[str]) -> list[str]:
    seen: set[str] = set()
    out: list[str] = []
    for item in items:
        if item in seen:
            continue
        seen.add(item)
        out.append(item)
    return out


# --------------------------------------------------------------------------- #
# Prompt rendering (provider-neutral)
# --------------------------------------------------------------------------- #
def render_prompt(task_json: dict[str, Any], *, next_target: Optional[str], workflow_intro: str) -> str:
    version = task_json["version"]
    kind = task_json["kind"]
    target = task_json["target"]
    unit = task_json.get("unit") or {}
    source = unit.get("source_path") or target.get("query")
    object_path = unit.get("object_path")
    ctx_path = unit.get("ctx_path")
    asm_path = unit.get("asm_path")

    lines: list[str] = [
        "# Decompilation Task",
        "",
        f"You are an autonomous decompilation agent for Crash Bandicoot: The Wrath "
        f"of Cortex (GameCube, version `{version}`). Work in small, verifiable "
        f"steps and do not ask questions; make best-effort progress.",
        "",
        "First read `AGENTS.md` and `.github/instructions/decomp.instructions.md` "
        "for the repo's hard rules.",
        "",
        "## Goal",
        "",
        workflow_intro,
        "",
        "## Target",
        "",
        f"- Query: `{target.get('query')}`",
    ]
    if target.get("function"):
        lines.append(f"- Function: `{target['function']}`")
    if target.get("address"):
        lines.append(f"- Address: `{target['address']}`")
    if target.get("size"):
        lines.append(f"- Size: `{target['size']}`")
    if target.get("current_match"):
        lines.append(f"- Current match: `{target['current_match']}`")
    if source:
        lines.append(f"- Source: `{source}`")
    if object_path:
        lines.append(f"- Object: `{object_path}`")
    if ctx_path:
        lines.append(f"- Context: `{ctx_path}`")
    if asm_path:
        lines.append(f"- Asm: `{asm_path}`")
    if next_target:
        lines.append(f"- Next target: {next_target}")

    lines += [
        "",
        "## Allowed edits",
        "",
    ]
    allowed = task_json["allowed_edit_files"]
    if allowed:
        lines += [f"- `{path}`" for path in allowed]
    else:
        lines.append("- (none resolved; resolve the owning source file before editing)")

    lines += [
        "",
        "## Forbidden edits",
        "",
        "Never hand-edit generated files:",
        "",
    ]
    lines += [f"- `{path}`" for path in task_json["forbidden_edit_files"]]

    lines += [
        "",
        "## Workflow",
        "",
        "1. Resolve context with the repo helpers before broad manual searching:",
        f"   - `python tools/ai_context.py {target.get('query')}`",
        f"   - `python tools/ai_match_plan.py {source}`" if source else
        "   - `python tools/ai_match_plan.py <unit>`",
        "   - `python tools/ai_lookup_symbol.py <symbol-or-address>`",
        f"   - `python tools/ai_lookup_unit.py {source}`" if source else
        "   - `python tools/ai_lookup_unit.py <unit>`",
        "2. Work on exactly one function at a time.",
        f"3. Build only the single object while iterating"
        + (f" (`ninja {object_path}`)." if object_path else "."),
        f"4. Build the `.ctx` when local context is thin"
        + (f" (`ninja {ctx_path}`)." if ctx_path else "."),
        "5. Inspect the extracted asm"
        + (f" (`{asm_path}`)" if asm_path else "")
        + " for instruction-level confirmation.",
        "6. Prefer true matches: typed fields and structured control flow. No inline asm.",
        "7. Never start an interactive `objdiff-cli diff` session.",
        "",
        "## Verification",
        "",
        "Run these one-shot commands (also in `verify.sh`):",
        "",
        "```sh",
    ]
    lines += task_json["verify_commands"]
    lines += [
        "```",
        "",
        "## Stop rules",
        "",
        "- One function at a time.",
        f"- Stop or mark blocked after "
        f"{task_json['stop_rules']['max_cycles_without_improvement']} cycles without improvement.",
        "- No inline asm.",
        "- No interactive objdiff.",
        "",
        "## Reporting format",
        "",
        "When finished, report:",
        "",
        "- functions fully matched",
        "- functions skipped or blocked (with the reason)",
        "- final match state",
        "- commands used for final verification",
        "",
    ]
    return "\n".join(lines) + "\n"


# --------------------------------------------------------------------------- #
# Context rendering
# --------------------------------------------------------------------------- #
def _commands_block(commands: list[str]) -> list[str]:
    if not commands:
        return []
    out = ["## Suggested commands", ""]
    out += [f"- `{command}`" for command in commands]
    out.append("")
    return out


def render_unit_context(unit: UnitInfo, version: str, category_names: dict[str, str]) -> str:
    counts = unit_match_counts(unit)
    next_function = next_unmatched_function(unit)
    interesting = ranked_unit_functions(unit, include_matched=False)[:8]
    if not interesting:
        interesting = ranked_unit_functions(unit)[:5]

    lines = [
        "# Context",
        "",
        "## Unit",
        "",
        f"- Name: `{unit.raw_name}`",
        f"- Normalized: `{unit.normalized_name}`",
        f"- Source: `{unit.source_path or '-'}`",
        f"- Object: `{derive_object_path(unit, version) or '-'}`",
        f"- Context (.ctx): `{derive_ctx_path(unit, version) or '-'}`",
        f"- Asm: `{derive_asm_path(unit, version) or '-'}`",
    ]
    if unit.build_label:
        lines.append(f"- Build label: `{unit.build_label}`")
    lines += [
        "",
        "## Match summary",
        "",
        f"- {counts['matched']}/{counts['total']} matched, {counts['remaining']} remaining "
        f"({counts['partial']} partial, {counts['unknown']} unknown)",
    ]
    if next_function is not None:
        lines += [
            "",
            "## Next unmatched function",
            "",
            f"- `{next_function.name}` at {format_hex(next_function.address)} "
            f"(size {format_size(next_function.size)}, match {format_percent(next_function.fuzzy_match_percent)})",
        ]
    if interesting:
        lines += ["", "## Functions", ""]
        for function in interesting:
            lines.append(
                f"- {format_hex(function.address)}  `{function.name}`  "
                f"size {format_size(function.size)}  match {format_percent(function.fuzzy_match_percent)}"
            )
    lines.append("")
    lines += _commands_block(dedupe([*suggest_unit_commands(unit), *suggest_regression_commands()]))
    return "\n".join(lines) + "\n"


def render_function_context(
    unit: Optional[UnitInfo],
    symbol,
    function,
    version: str,
    category_names: dict[str, str],
) -> str:
    lines = ["# Context", ""]
    if symbol is not None:
        lines += [
            "## Symbol",
            "",
            f"- Name: `{symbol.name}`",
            f"- Section: `{symbol.section}`",
            f"- Address: {format_hex(symbol.address)}",
            f"- Size: {format_size(symbol.size)}",
        ]
        if symbol.scope:
            lines.append(f"- Scope: `{symbol.scope}`")
        if symbol.symbol_type:
            lines.append(f"- Type: `{symbol.symbol_type}`")
        lines.append("")
    if function is not None:
        lines += [
            "## Report function",
            "",
            f"- `{function.name}` at {format_hex(function.address)} "
            f"(size {format_size(function.size)}, current match {format_percent(function.fuzzy_match_percent)})",
            "",
        ]
    if unit is not None:
        lines += [
            "## Owning unit",
            "",
            f"- Name: `{unit.raw_name}`",
            f"- Source: `{unit.source_path or '-'}`",
            f"- Object: `{derive_object_path(unit, version) or '-'}`",
            f"- Context (.ctx): `{derive_ctx_path(unit, version) or '-'}`",
            f"- Asm: `{derive_asm_path(unit, version) or '-'}`",
            "",
        ]
        address = function.address if function is not None else (symbol.address if symbol is not None else None)
        nearby = nearby_functions(unit, address) if address is not None else []
        if nearby:
            lines += ["## Nearby functions", ""]
            for entry in nearby:
                lines.append(
                    f"- {format_hex(entry.address)}  `{entry.name}`  "
                    f"size {format_size(entry.size)}  match {format_percent(entry.fuzzy_match_percent)}"
                )
            lines.append("")

    commands: list[str] = []
    if symbol is not None:
        commands.append(f"python tools/ai_lookup_symbol.py {symbol.name}")
    if unit is not None:
        commands += suggest_unit_commands(unit)
    commands += suggest_regression_commands()
    lines += _commands_block(dedupe(commands))
    return "\n".join(lines) + "\n"


def render_decompme_context(bundle, payload: dict[str, Any]) -> str:
    meta = payload["bundle"]["metadata"]
    lines = [
        "# Context",
        "",
        "## Bundle",
        "",
        f"- Name: `{meta.get('name', bundle.path.stem)}`",
        f"- Path: `{payload['bundle']['path']}`",
        f"- Source kind: `{payload['bundle']['source_kind']}`",
    ]
    if meta.get("compiler"):
        lines.append(f"- Compiler: `{meta['compiler']}`")
    if meta.get("platform"):
        lines.append(f"- Platform: `{meta['platform']}`")
    function_names = payload["bundle"]["function_names"]
    if function_names:
        lines += ["", "## Snippet functions", ""]
        lines += [f"- `{name}`" for name in function_names]

    if payload["functions"]:
        lines += ["", "## Resolved targets", ""]
        for resolved in payload["functions"]:
            unit = resolved["unit"]
            hint = resolved["placement_hint"]
            lines.append(f"- `{resolved['name']}`")
            if unit is not None:
                lines.append(f"  - Unit: `{unit['normalized_name']}`")
                if unit.get("source_path"):
                    lines.append(f"  - Source: `{unit['source_path']}`")
            if resolved["report_function"] is not None:
                lines.append(
                    f"  - Match: {format_percent(resolved['report_function']['fuzzy_match_percent'])}"
                )
            if hint is not None:
                lines.append(f"  - Placement: {hint['message']}")

    missing = payload.get("missing_declarations_by_unit") or {}
    rendered_missing = {unit: decls for unit, decls in missing.items() if decls}
    if rendered_missing:
        lines += ["", "## Candidate missing declarations", ""]
        for unit_name, decls in rendered_missing.items():
            lines.append(f"- `{unit_name}`")
            for decl in decls[:16]:
                lines.append(f"  - {decl['kind']}: `{decl['name']}`")

    lines.append("")
    lines += _commands_block(dedupe(payload.get("commands", [])))
    return "\n".join(lines) + "\n"


# --------------------------------------------------------------------------- #
# Task builders
# --------------------------------------------------------------------------- #
def build_unit_task(query: str, version: str) -> TaskPlan:
    matches = find_units(query, version=version)
    if not matches:
        raise TaskError(
            f"No unit matches found for '{query}'.\n"
            f"Try: python tools/ai_lookup_unit.py {query}"
        )
    unit = matches[0]
    category_names = load_repo_index(version).category_names

    task_json = {
        "schema_version": SCHEMA_VERSION,
        "kind": "match_unit",
        "version": version,
        "target": {"query": query},
        "unit": unit_block(unit, version),
        "allowed_edit_files": allowed_edit_files(unit),
        "forbidden_edit_files": forbidden_edit_files(version),
        "verify_commands": verify_commands(unit, version),
        "stop_rules": STOP_RULES,
    }

    next_function = next_unmatched_function(unit)
    next_target = None
    if next_function is not None:
        next_target = (
            f"`{next_function.name}` at {format_hex(next_function.address)} "
            f"(match {format_percent(next_function.fuzzy_match_percent)})"
        )

    prompt_md = render_prompt(
        task_json,
        next_target=next_target,
        workflow_intro=f"Match every reachable function in `{unit.source_path or unit.normalized_name}` "
        f"to 100%, one function at a time, without regressing other units.",
    )
    context_md = render_unit_context(unit, version, category_names)
    return TaskPlan(
        kind="match_unit",
        slug=slugify(unit.source_path or unit.normalized_name),
        target_label=unit.source_path or unit.normalized_name,
        task_json=task_json,
        prompt_md=prompt_md,
        context_md=context_md,
    )


def build_function_task(query: str, version: str) -> TaskPlan:
    kind, unit, symbol, function = resolve_context(query, version)
    if unit is None and symbol is None and function is None:
        raise TaskError(
            f"No symbol, function, or unit matched '{query}'.\n"
            f"Try: python tools/ai_lookup_symbol.py {query}"
        )
    category_names = load_repo_index(version).category_names

    target: dict[str, Any] = {"query": query}
    if function is not None:
        target["function"] = function.name
        target["address"] = format_hex(function.address)
        target["size"] = format_size(function.size)
        target["current_match"] = (
            format_percent(function.fuzzy_match_percent)
            if function.fuzzy_match_percent is not None
            else None
        )
    elif symbol is not None:
        target["function"] = symbol.name
        target["address"] = format_hex(symbol.address)
        target["size"] = format_size(symbol.size)

    task_json = {
        "schema_version": SCHEMA_VERSION,
        "kind": "match_function",
        "version": version,
        "target": target,
        "unit": unit_block(unit, version),
        "allowed_edit_files": allowed_edit_files(unit),
        "forbidden_edit_files": forbidden_edit_files(version),
        "verify_commands": verify_commands(unit, version),
        "stop_rules": STOP_RULES,
    }

    label = target.get("function") or query
    prompt_md = render_prompt(
        task_json,
        next_target=None,
        workflow_intro=f"Match the function `{label}` to 100% without regressing the rest of its unit.",
    )
    context_md = render_function_context(unit, symbol, function, version, category_names)
    return TaskPlan(
        kind="match_function",
        slug=slugify(label),
        target_label=label,
        task_json=task_json,
        prompt_md=prompt_md,
        context_md=context_md,
    )


def build_decompme_task(target: str, version: str) -> TaskPlan:
    bundle_path = Path(target)
    if not bundle_path.exists():
        raise TaskError(f"Bundle path does not exist: {bundle_path}")
    try:
        bundle = ai_decompme_zip.read_bundle(bundle_path)
    except (FileNotFoundError, json.JSONDecodeError, zipfile.BadZipFile) as exc:
        raise TaskError(f"Failed to read bundle: {exc}")

    function_names = ai_decompme_zip.iter_function_names(bundle.code_text)
    resolved = [ai_decompme_zip.resolve_function(name, version) for name in function_names]
    primary = next((item for item in resolved if item["unit"] is not None), None)
    if primary is None and resolved:
        primary = resolved[0]
    unit = primary["unit"] if primary else None

    payload = ai_decompme_zip.build_payload(bundle, resolved)

    bundle_name = bundle.metadata.get("name") or bundle_path.stem
    primary_name = primary["name"] if primary else None
    task_json = {
        "schema_version": SCHEMA_VERSION,
        "kind": "match_decompme",
        "version": version,
        "target": {
            "query": str(bundle_path),
            "bundle": bundle_name,
            "functions": function_names,
            "primary_function": primary_name,
        },
        "unit": unit_block(unit, version),
        "allowed_edit_files": allowed_edit_files(unit),
        "forbidden_edit_files": forbidden_edit_files(version),
        "verify_commands": verify_commands(unit, version),
        "stop_rules": STOP_RULES,
    }

    workflow_intro = (
        f"Integrate the decomp.me snippet from `{bundle_name}` into the repo and match "
        "the affected function(s). Add any missing declarations and place each function "
        "using the placement hints in context.md."
    )
    prompt_md = render_prompt(task_json, next_target=None, workflow_intro=workflow_intro)
    context_md = render_decompme_context(bundle, payload)
    return TaskPlan(
        kind="match_decompme",
        slug=slugify(primary_name or bundle_name),
        target_label=bundle_name,
        task_json=task_json,
        prompt_md=prompt_md,
        context_md=context_md,
    )


# --------------------------------------------------------------------------- #
# Writing
# --------------------------------------------------------------------------- #
def render_verify_sh(commands: list[str]) -> str:
    lines = ["#!/usr/bin/env sh", "set -eu", ""]
    lines += commands
    return "\n".join(lines) + "\n"


def _write(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8", newline="\n")


def write_task_pack(plan: TaskPlan, out_dir: Path, force: bool) -> Path:
    timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    task_dir = out_dir / f"{timestamp}-{plan.slug}"
    if task_dir.exists() and not force:
        raise TaskError(
            f"Task directory already exists: {display_path(task_dir)} (use --force to overwrite)."
        )
    task_dir.mkdir(parents=True, exist_ok=True)
    _write(task_dir / "task.json", json.dumps(plan.task_json, indent=2) + "\n")
    _write(task_dir / "prompt.md", plan.prompt_md)
    _write(task_dir / "context.md", plan.context_md)
    _write(task_dir / "verify.sh", render_verify_sh(plan.task_json["verify_commands"]))
    _write(task_dir / "notes.md", NOTES_TEMPLATE)
    return task_dir


def build_plan(mode: str, target: str, version: str) -> TaskPlan:
    if mode == "unit":
        return build_unit_task(target, version)
    if mode == "function":
        return build_function_task(target, version)
    if mode == "decompme":
        return build_decompme_task(target, version)
    raise TaskError(f"Unknown task mode: {mode}")


# --------------------------------------------------------------------------- #
# CLI
# --------------------------------------------------------------------------- #
def build_parser() -> argparse.ArgumentParser:
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        help=f"Project version under config/ and build/ (default: {DEFAULT_VERSION})",
    )
    common.add_argument(
        "--out-dir",
        default=str(DEFAULT_OUT_DIR),
        help="Directory that holds generated task packs (default: .ai/tasks)",
    )
    common.add_argument("--print-path", action="store_true", help="Print only the task path.")
    common.add_argument("--json", action="store_true", help="Print a JSON summary of the task pack.")
    common.add_argument("--force", action="store_true", help="Overwrite an existing task directory.")

    parser = argparse.ArgumentParser(
        description="Generate a provider-neutral decompilation task pack.",
    )
    sub = parser.add_subparsers(dest="mode", required=True)
    p_unit = sub.add_parser("unit", parents=[common], help="Task pack for a unit or source file.")
    p_unit.add_argument("target", help="Unit name or source path, e.g. src/gamecode/crate.c")
    p_function = sub.add_parser("function", parents=[common], help="Task pack for a function or address.")
    p_function.add_argument("target", help="Symbol name or address, e.g. MoveCrate or 0x800032A0")
    p_decompme = sub.add_parser("decompme", parents=[common], help="Task pack from a decomp.me export.")
    p_decompme.add_argument("target", help="Path to a .zip export or extracted directory")
    return parser


def main(argv: Optional[list[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    try:
        plan = build_plan(args.mode, args.target, args.version)
        task_dir = write_task_pack(plan, Path(args.out_dir), args.force)
    except TaskError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    shown = display_path(task_dir)
    if args.print_path:
        print(shown)
        return 0
    if args.json:
        summary = {
            "task_path": shown,
            "kind": plan.kind,
            "target": plan.target_label,
            "files": [f"{shown}/{name}" for name in TASK_FILES],
        }
        print(json.dumps(summary, indent=2))
        return 0

    print(f"Created task pack: {shown}")
    print(f"Files: {', '.join(TASK_FILES)}")
    print(f"Next: python tools/ai_run.py --backend print {shown}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
