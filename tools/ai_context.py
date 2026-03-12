#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from typing import Any

from ai_common import (
    DEFAULT_VERSION,
    find_report_functions_by_address,
    find_report_functions_by_name,
    find_symbols_by_address,
    find_symbols_by_name,
    find_units,
    find_units_by_address,
    format_hex,
    format_percent,
    format_size,
    load_repo_index,
    nearby_functions,
    parse_address,
    progress_category_labels,
    suggest_regression_commands,
    suggest_unit_commands,
    unit_summary,
)


def interesting_functions(unit):
    if not unit.functions:
        return []
    non_matching = [
        function
        for function in unit.functions
        if function.fuzzy_match_percent is None or function.fuzzy_match_percent < 100.0
    ]
    if non_matching:
        return sorted(
            non_matching,
            key=lambda function: (
                function.fuzzy_match_percent if function.fuzzy_match_percent is not None else -1.0,
                function.address,
            ),
        )[:8]
    return unit.functions[:5]


def dedupe(items):
    out = []
    seen = set()
    for item in items:
        if item in seen:
            continue
        seen.add(item)
        out.append(item)
    return out


def build_symbol_payload(unit, symbol, function, category_names: dict[str, str]) -> dict[str, Any]:
    return {
        "kind": "symbol",
        "symbol": {
            "name": symbol.name,
            "section": symbol.section,
            "address": symbol.address,
            "size": symbol.size,
            "scope": symbol.scope,
            "symbol_type": symbol.symbol_type,
        },
        "unit": unit_summary(unit, category_names),
        "function": {
            "name": function.name,
            "address": function.address,
            "size": function.size,
            "fuzzy_match_percent": function.fuzzy_match_percent,
        }
        if function is not None
        else None,
        "nearby_functions": [
            {
                "name": nearby.name,
                "address": nearby.address,
                "size": nearby.size,
                "fuzzy_match_percent": nearby.fuzzy_match_percent,
            }
            for nearby in nearby_functions(unit, symbol.address)
        ],
        "commands": [
            *dedupe(
                [
                    f"python tools/ai_lookup_symbol.py {symbol.name}",
                    f"python tools/ai_lookup_unit.py {unit.normalized_name}",
                    *suggest_unit_commands(unit),
                    *suggest_regression_commands(),
                ]
            )
        ],
    }


def build_unit_payload(unit, category_names: dict[str, str]) -> dict[str, Any]:
    return {
        "kind": "unit",
        "unit": unit_summary(unit, category_names),
        "interesting_functions": [
            {
                "name": function.name,
                "address": function.address,
                "size": function.size,
                "fuzzy_match_percent": function.fuzzy_match_percent,
            }
            for function in interesting_functions(unit)
        ],
        "commands": [*dedupe([*suggest_unit_commands(unit), *suggest_regression_commands()])],
    }


def print_symbol_context(unit, symbol, function, category_names: dict[str, str]) -> None:
    print("Kind: symbol")
    print(f"Symbol: {symbol.name}")
    print(f"Section: {symbol.section}")
    print(f"Address: {format_hex(symbol.address)}")
    print(f"Size: {format_size(symbol.size)}")
    if symbol.scope:
        print(f"Scope: {symbol.scope}")
    if symbol.symbol_type:
        print(f"Type: {symbol.symbol_type}")
    print(f"Unit: {unit.raw_name}")
    if unit.source_path:
        print(f"Source: {unit.source_path}")
    if unit.object_path:
        print(f"Object: {unit.object_path}")
    if unit.ctx_path:
        print(f"Context: {unit.ctx_path}")
    labels = progress_category_labels(unit, category_names)
    if labels:
        print(f"Progress categories: {', '.join(labels)}")

    if function is not None:
        print(f"Function match: {format_percent(function.fuzzy_match_percent)}")

    nearby = nearby_functions(unit, symbol.address)
    if nearby:
        print("Nearby functions:")
        for entry in nearby:
            print(
                f"  {format_hex(entry.address)}  {entry.name:<32} "
                f"size {format_size(entry.size)}  match {format_percent(entry.fuzzy_match_percent)}"
            )

    print("Commands:")
    for command in dedupe(
        [
            f"python tools/ai_lookup_symbol.py {symbol.name}",
            f"python tools/ai_lookup_unit.py {unit.normalized_name}",
            *suggest_unit_commands(unit),
            *suggest_regression_commands(),
        ]
    ):
        print(f"  {command}")


def print_unit_context(unit, category_names: dict[str, str]) -> None:
    print("Kind: unit")
    print(f"Unit: {unit.raw_name}")
    if unit.raw_name != unit.normalized_name:
        print(f"Normalized: {unit.normalized_name}")
    if unit.source_path:
        print(f"Source: {unit.source_path}")
    if unit.object_path:
        print(f"Object: {unit.object_path}")
    if unit.ctx_path:
        print(f"Context: {unit.ctx_path}")
    if unit.build_label:
        print(f"Build label: {unit.build_label}")
    labels = progress_category_labels(unit, category_names)
    if labels:
        print(f"Progress categories: {', '.join(labels)}")

    if unit.measures:
        matched_code = unit.measures.get("matched_code_percent")
        matched_functions = unit.measures.get("matched_functions_percent")
        total_functions = unit.measures.get("total_functions")
        matched_functions_count = unit.measures.get("matched_functions")
        if matched_code is not None:
            print(f"Matched code: {format_percent(float(matched_code))}")
        if matched_functions is not None:
            count_text = ""
            if matched_functions_count is not None and total_functions is not None:
                count_text = f" ({matched_functions_count}/{total_functions})"
            print(f"Matched functions: {format_percent(float(matched_functions))}{count_text}")

    if unit.sections:
        print("Sections:")
        for section in unit.sections:
            print(
                f"  {section.section:<7} {format_hex(section.start)} - {format_hex(section.end)}"
            )

    interesting = interesting_functions(unit)
    if interesting:
        print("Interesting functions:")
        for function in interesting:
            print(
                f"  {format_hex(function.address)}  {function.name:<32} "
                f"size {format_size(function.size)}  match {format_percent(function.fuzzy_match_percent)}"
            )

    print("Commands:")
    for command in dedupe([*suggest_unit_commands(unit), *suggest_regression_commands()]):
        print(f"  {command}")


def resolve_context(query: str, version: str):
    address = parse_address(query)
    if address is not None:
        units = find_units_by_address(address, version=version)
        symbols = find_symbols_by_address(address, version=version)
        functions = find_report_functions_by_address(address, version=version)
        if symbols and units:
            unit = units[0]
            symbol = symbols[0]
            function = functions[0][1] if functions else None
            return ("symbol", unit, symbol, function)
        if units:
            return ("unit", units[0], None, None)
        return (None, None, None, None)

    looks_like_unit = any(
        query.endswith(suffix) for suffix in (".c", ".s", ".cpp", ".o", ".ctx")
    ) or "/" in query or "\\" in query
    exact_unit_matches = find_units(query, version=version, exact=True)
    exact_symbol_matches = find_symbols_by_name(query, version=version, exact=True)
    exact_function_matches = find_report_functions_by_name(query, version=version, exact=True)

    if looks_like_unit and exact_unit_matches:
        return ("unit", exact_unit_matches[0], None, None)
    if exact_symbol_matches and exact_function_matches:
        unit, function = exact_function_matches[0]
        return ("symbol", unit, exact_symbol_matches[0], function)
    if exact_symbol_matches:
        unit_matches = find_units_by_address(exact_symbol_matches[0].address, version=version)
        unit = unit_matches[0] if unit_matches else None
        function = exact_function_matches[0][1] if exact_function_matches else None
        if unit is not None:
            return ("symbol", unit, exact_symbol_matches[0], function)
    if exact_unit_matches:
        return ("unit", exact_unit_matches[0], None, None)

    unit_matches = find_units(query, version=version)
    if looks_like_unit and unit_matches:
        return ("unit", unit_matches[0], None, None)

    symbol_matches = find_symbols_by_name(query, version=version)
    function_matches = find_report_functions_by_name(query, version=version)
    if symbol_matches and function_matches:
        unit, function = function_matches[0]
        return ("symbol", unit, symbol_matches[0], function)
    if unit_matches:
        return ("unit", unit_matches[0], None, None)
    return (None, None, None, None)


def main() -> int:
    parser = argparse.ArgumentParser(description="Build a combined symbol or unit context summary.")
    parser.add_argument("query", help="Symbol name, unit name, path, or address")
    parser.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        help=f"Project version under config/ and build/ (default: {DEFAULT_VERSION})",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Emit machine-readable JSON",
    )
    args = parser.parse_args()

    index = load_repo_index(args.version)
    kind, unit, symbol, function = resolve_context(args.query, args.version)
    if kind is None or unit is None:
        print(f"No context found for '{args.query}'.")
        return 1

    if args.json:
        if kind == "symbol" and symbol is not None:
            payload = build_symbol_payload(unit, symbol, function, index.category_names)
        else:
            payload = build_unit_payload(unit, index.category_names)
        print(json.dumps(payload, indent=2))
        return 0

    if kind == "symbol" and symbol is not None:
        print_symbol_context(unit, symbol, function, index.category_names)
    else:
        print_unit_context(unit, index.category_names)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
