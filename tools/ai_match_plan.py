#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from typing import Any

from ai_common import (
    DEFAULT_VERSION,
    find_units,
    format_hex,
    format_percent,
    format_size,
    function_match_status,
    load_repo_index,
    next_unmatched_function,
    ranked_unit_functions,
    suggest_regression_commands,
    suggest_unit_commands,
    unit_asm_path,
    unit_match_counts,
    unit_summary,
)


def dedupe(items: list[str]) -> list[str]:
    out: list[str] = []
    seen: set[str] = set()
    for item in items:
        if item in seen:
            continue
        seen.add(item)
        out.append(item)
    return out


def function_payload(function) -> dict[str, Any]:
    return {
        "name": function.name,
        "address": function.address,
        "size": function.size,
        "fuzzy_match_percent": function.fuzzy_match_percent,
        "status": function_match_status(function),
    }


def build_plan_payload(
    query: str,
    unit,
    category_names: dict[str, str],
    functions,
    version: str = DEFAULT_VERSION,
) -> dict[str, Any]:
    next_function = next_unmatched_function(unit)
    commands = list(suggest_unit_commands(unit))
    if next_function is not None:
        commands.extend(
            [
                f"python tools/ai_lookup_symbol.py {next_function.name}",
                f"python tools/ai_context.py {next_function.name}",
            ]
        )
    commands.extend(suggest_regression_commands())

    return {
        "query": query,
        "unit": unit_summary(unit, category_names),
        "asm_path": unit_asm_path(unit, version=version),
        "match_summary": unit_match_counts(unit),
        "next_function": function_payload(next_function) if next_function is not None else None,
        "functions": [function_payload(function) for function in functions],
        "commands": dedupe(commands),
        "workflow": [
            "Resolve the unit and keep one active function at a time.",
            "Build the context or inspect the extracted unit asm when source context is not enough.",
            "Edit narrowly, build the single object, then re-run this plan or ai_lookup_symbol to measure progress.",
            "If match percentage does not improve after 4 build/measure cycles, record the blocker and move to the next function.",
            "Use ninja changes before wrapping up if matching or progress may have changed.",
        ],
    }


def print_plan(query: str, unit, functions, version: str = DEFAULT_VERSION) -> None:
    print(f"Query: {query}")
    print(f"Unit: {unit.raw_name}")
    if unit.raw_name != unit.normalized_name:
        print(f"Normalized: {unit.normalized_name}")
    if unit.source_path:
        print(f"Source: {unit.source_path}")
    if unit.object_path:
        print(f"Object: {unit.object_path}")
    if unit.ctx_path:
        print(f"Context: {unit.ctx_path}")
    print(f"Asm: {unit_asm_path(unit, version=version)}")

    counts = unit_match_counts(unit)
    if counts["total"]:
        print(
            "Match summary: "
            f"{counts['matched']}/{counts['total']} matched, "
            f"{counts['remaining']} remaining "
            f"({counts['partial']} partial, {counts['unknown']} unknown)"
        )
    else:
        print("Match summary: no report functions available yet")

    next_function = next_unmatched_function(unit)
    if next_function is not None:
        print(
            f"Next target: {next_function.name}  {format_hex(next_function.address)}  "
            f"size {format_size(next_function.size)}  "
            f"match {format_percent(next_function.fuzzy_match_percent)}"
        )
    elif unit.functions:
        print("Next target: all reported functions are already at 100%")

    if functions:
        print("Open functions:")
        for function in functions:
            print(
                f"  {format_hex(function.address)}  {function.name:<32} "
                f"size {format_size(function.size)}  "
                f"match {format_percent(function.fuzzy_match_percent)}  "
                f"[{function_match_status(function)}]"
            )
    elif unit.functions:
        print("Open functions: none")

    print("Commands:")
    for command in dedupe(
        [
            *suggest_unit_commands(unit),
            *(
                [
                    f"python tools/ai_lookup_symbol.py {next_function.name}",
                    f"python tools/ai_context.py {next_function.name}",
                ]
                if next_function is not None
                else []
            ),
            *suggest_regression_commands(),
        ]
    ):
        print(f"  {command}")

    print("Loop:")
    print("  1. Work one function at a time, starting with the next target above.")
    print("  2. Re-run this command after each measurable change to refresh the backlog.")
    print("  3. If a function stops improving after 4 build/measure cycles, move on and note why.")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Plan a file- or unit-based matching campaign from report metadata."
    )
    parser.add_argument("query", help="Unit name, source path, or other ai_lookup_unit query")
    parser.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        help=f"Project version under config/ and build/ (default: {DEFAULT_VERSION})",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=12,
        help="Maximum number of functions to print (default: 12, ignored by --all)",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Include already-matched functions too",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Emit machine-readable JSON",
    )
    args = parser.parse_args()

    index = load_repo_index(args.version)
    matches = find_units(args.query, version=args.version)
    if not matches:
        print(f"No unit matches found for '{args.query}'.")
        return 1

    unit = matches[0]
    functions = ranked_unit_functions(unit, include_matched=args.all)
    if not args.all and args.limit >= 0:
        functions = functions[: args.limit]

    if args.json:
        payload = build_plan_payload(
            args.query, unit, index.category_names, functions, version=args.version
        )
        if len(matches) > 1:
            payload["resolved_from_multiple_matches"] = [
                candidate.normalized_name for candidate in matches[:8]
            ]
        print(json.dumps(payload, indent=2))
        return 0

    if len(matches) > 1:
        print(
            "Resolved from multiple unit matches: "
            + ", ".join(candidate.normalized_name for candidate in matches[:5])
        )

    print_plan(args.query, unit, functions, version=args.version)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
