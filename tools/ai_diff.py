#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ai_common import (
    DEFAULT_VERSION,
    find_units,
    format_hex,
    format_percent,
    format_size,
    function_match_status,
    load_repo_index,
    ranked_unit_functions,
    unit_asm_path,
    unit_match_counts,
)


def safe_int(value: Any) -> int | None:
    if value is None:
        return None
    if isinstance(value, int):
        return value
    return int(str(value))


def safe_float(value: Any) -> float | None:
    if value is None:
        return None
    return float(value)


def load_changes(version: str = DEFAULT_VERSION) -> dict[str, Any] | None:
    path = Path("build") / version / "report_changes.json"
    if not path.is_file():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def find_unit_changes(changes: dict[str, Any] | None, report_unit_name: str | None) -> dict[str, Any] | None:
    if changes is None or not report_unit_name:
        return None
    for unit in changes.get("units", []):
        if unit.get("name") == report_unit_name:
            return unit
    return None


def build_change_map(unit_changes: dict[str, Any] | None) -> dict[int, dict[str, Any]]:
    if unit_changes is None:
        return {}
    out: dict[int, dict[str, Any]] = {}
    for function_change in unit_changes.get("functions", []):
        address = safe_int(function_change.get("metadata", {}).get("virtual_address"))
        if address is None:
            continue
        out[address] = function_change
    return out


def build_function_change_payload(function_change: dict[str, Any] | None) -> dict[str, Any] | None:
    if function_change is None:
        return None

    from_data = function_change.get("from", {})
    to_data = function_change.get("to", {})
    from_match = safe_float(from_data.get("fuzzy_match_percent"))
    to_match = safe_float(to_data.get("fuzzy_match_percent"))
    delta_match = None
    if from_match is not None and to_match is not None:
        delta_match = to_match - from_match
    return {
        "from_fuzzy_match_percent": from_match,
        "to_fuzzy_match_percent": to_match,
        "delta_fuzzy_match_percent": delta_match,
        "from_size": safe_int(from_data.get("size")),
        "to_size": safe_int(to_data.get("size")),
    }


def build_function_payload(function, function_change: dict[str, Any] | None = None) -> dict[str, Any]:
    payload = {
        "name": function.name,
        "address": function.address,
        "size": function.size,
        "fuzzy_match_percent": function.fuzzy_match_percent,
        "status": function_match_status(function),
    }
    change_payload = build_function_change_payload(function_change)
    if change_payload is not None:
        payload["change"] = change_payload
    return payload


def build_unit_payload(
    unit,
    version: str,
    include_changes: bool,
    functions_override=None,
    unit_changes: dict[str, Any] | None = None,
) -> dict[str, Any]:
    change_map = build_change_map(unit_changes)
    functions = functions_override if functions_override is not None else ranked_unit_functions(unit)
    payload = {
        "unit": unit.normalized_name,
        "source_path": unit.source_path,
        "report_unit_name": unit.report_unit_name,
        "asm_path": unit_asm_path(unit, version=version),
        "match_summary": unit_match_counts(unit),
        "functions": [
            build_function_payload(function, change_map.get(function.address)) for function in functions
        ],
    }
    if include_changes:
        payload["changes"] = unit_changes
    return payload


def format_unit_change_summary(unit_changes: dict[str, Any] | None) -> str:
    if unit_changes is None:
        return "no current entry for this unit in report_changes.json"

    from_functions = safe_float(unit_changes.get("from", {}).get("matched_functions_percent"))
    to_functions = safe_float(unit_changes.get("to", {}).get("matched_functions_percent"))
    from_code = safe_float(unit_changes.get("from", {}).get("matched_code_percent"))
    to_code = safe_float(unit_changes.get("to", {}).get("matched_code_percent"))

    parts: list[str] = []
    if from_functions is not None and to_functions is not None:
        parts.append(f"functions {format_percent(from_functions)} -> {format_percent(to_functions)}")
    elif to_functions is not None:
        parts.append(f"functions now {format_percent(to_functions)}")
    if from_code is not None and to_code is not None:
        parts.append(f"code {format_percent(from_code)} -> {format_percent(to_code)}")
    elif to_code is not None:
        parts.append(f"code now {format_percent(to_code)}")
    return ", ".join(parts) if parts else "reported, but no percent deltas are available"


def format_function_change_summary(function_change: dict[str, Any] | None) -> str:
    change_payload = build_function_change_payload(function_change)
    if change_payload is None:
        return ""

    from_match = change_payload["from_fuzzy_match_percent"]
    to_match = change_payload["to_fuzzy_match_percent"]
    delta_match = change_payload["delta_fuzzy_match_percent"]
    if from_match is not None and to_match is not None and delta_match is not None:
        return f"Δ {delta_match:+.2f}pp ({format_percent(from_match)} -> {format_percent(to_match)})"
    if to_match is not None:
        return f"reported now {format_percent(to_match)}"
    if from_match is not None:
        return f"reported from {format_percent(from_match)}"

    from_size = change_payload["from_size"]
    to_size = change_payload["to_size"]
    if from_size is not None or to_size is not None:
        return f"size {format_size(from_size)} -> {format_size(to_size)}"
    return "reported"


def print_unit_overview(
    unit,
    functions,
    version: str,
    unit_changes: dict[str, Any] | None,
    include_changes: bool,
) -> None:
    print(f"Unit: {unit.normalized_name}")
    if unit.source_path:
        print(f"Source: {unit.source_path}")
    if unit.report_unit_name:
        print(f"Report unit: {unit.report_unit_name}")
    print(f"Asm: {unit_asm_path(unit, version=version)}")

    counts = unit_match_counts(unit)
    print(
        "Match summary: "
        f"{counts['matched']}/{counts['total']} matched, "
        f"{counts['remaining']} remaining "
        f"({counts['partial']} partial, {counts['unknown']} unknown)"
    )

    if include_changes:
        print(f"Changes: {format_unit_change_summary(unit_changes)}")

    change_map = build_change_map(unit_changes)
    print("Functions:")
    for function in functions:
        change_text = ""
        if include_changes:
            summary = format_function_change_summary(change_map.get(function.address))
            if summary:
                change_text = f"  {summary}"
        print(
            f"  {format_hex(function.address)}  {function.name:<32} "
            f"size {format_size(function.size)}  match {format_percent(function.fuzzy_match_percent)}  "
            f"[{function_match_status(function)}]{change_text}"
        )


def print_function_change(function_changes: dict[str, Any] | None) -> None:
    if function_changes is None:
        return
    print(f"    {format_function_change_summary(function_changes)}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Show report-driven function and unit diff summaries without launching interactive objdiff."
    )
    parser.add_argument("unit", help="Unit name or source path such as crate.c or src/gamecode/crate.c")
    parser.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        help=f"Project version under config/ and build/ (default: {DEFAULT_VERSION})",
    )
    parser.add_argument(
        "--function",
        default=None,
        help="Optional function name substring to filter within the unit.",
    )
    parser.add_argument(
        "--status",
        choices=["matched", "partial", "unknown"],
        default=None,
        help="Filter functions by match status.",
    )
    parser.add_argument(
        "--changes",
        action="store_true",
        help="Include any available data from build/<version>/report_changes.json.",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=20,
        help="Maximum number of functions to print (default: 20). Use -1 for all.",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Emit machine-readable JSON.",
    )
    args = parser.parse_args()

    _ = load_repo_index(args.version)
    matches = find_units(args.unit, version=args.version)
    if not matches:
        print(f"No unit matches found for '{args.unit}'.")
        return 1

    unit = matches[0]
    functions = ranked_unit_functions(unit)
    if args.function:
        query = args.function.lower()
        functions = [function for function in functions if query in function.name.lower()]
    if args.status:
        functions = [function for function in functions if function_match_status(function) == args.status]
    if args.limit >= 0:
        functions = functions[: args.limit]

    changes_data = load_changes(args.version) if args.changes else None
    unit_changes = find_unit_changes(changes_data, unit.report_unit_name)

    if args.json:
        payload = build_unit_payload(
            unit,
            args.version,
            include_changes=args.changes,
            functions_override=functions,
            unit_changes=unit_changes,
        )
        print(json.dumps(payload, indent=2))
        return 0

    print_unit_overview(unit, functions, args.version, unit_changes, include_changes=args.changes)
    if unit_changes is not None and args.function:
        query = args.function.lower()
        change_functions = unit_changes.get("functions", [])
        print("Filtered function changes:")
        for function_change in change_functions:
            name = function_change.get("name", "")
            if query not in name.lower():
                continue
            print(f"  {name}")
            print_function_change(function_change)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
