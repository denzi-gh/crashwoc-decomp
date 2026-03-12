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
    find_units_by_address,
    format_hex,
    format_percent,
    format_size,
    load_repo_index,
    parse_address,
    search_dwarf_lines,
)


def symbol_payload(symbol) -> dict[str, Any]:
    return {
        "name": symbol.name,
        "section": symbol.section,
        "address": symbol.address,
        "size": symbol.size,
        "scope": symbol.scope,
        "symbol_type": symbol.symbol_type,
        "comment": symbol.comment,
    }


def function_payload(unit, function) -> dict[str, Any]:
    return {
        "name": function.name,
        "address": function.address,
        "size": function.size,
        "fuzzy_match_percent": function.fuzzy_match_percent,
        "unit": unit.normalized_name,
        "source_path": unit.source_path,
        "object_path": unit.object_path,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Lookup symbols, addresses, and function match data.")
    parser.add_argument("query", help="Symbol name or address")
    parser.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        help=f"Project version under config/ and build/ (default: {DEFAULT_VERSION})",
    )
    parser.add_argument(
        "--exact",
        action="store_true",
        help="Only return exact text matches for non-address queries",
    )
    parser.add_argument(
        "--dwarf",
        action="store_true",
        help="Include matching lines from src/dump_alphaNGCport_DWARF.txt",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Emit machine-readable JSON",
    )
    args = parser.parse_args()

    _ = load_repo_index(args.version)
    address = parse_address(args.query)
    if address is not None:
        symbol_hits = find_symbols_by_address(address, version=args.version)
        function_hits = find_report_functions_by_address(address, version=args.version)
        unit_hits = find_units_by_address(address, version=args.version)
        dwarf_hits = search_dwarf_lines(format_hex(address), limit=5, version=args.version)
    else:
        symbol_hits = find_symbols_by_name(args.query, version=args.version, exact=args.exact)
        function_hits = find_report_functions_by_name(args.query, version=args.version, exact=args.exact)
        unit_hits = []
        if args.dwarf or (not symbol_hits and not function_hits):
            dwarf_hits = search_dwarf_lines(args.query, limit=5, version=args.version)
        else:
            dwarf_hits = []

    if args.json:
        payload = {
            "query": args.query,
            "symbols": [symbol_payload(symbol) for symbol in symbol_hits],
            "functions": [function_payload(unit, function) for unit, function in function_hits],
            "units": [unit.normalized_name for unit in unit_hits],
            "dwarf_lines": dwarf_hits,
        }
        print(json.dumps(payload, indent=2))
        return 0 if (symbol_hits or function_hits or unit_hits or dwarf_hits) else 1

    if not symbol_hits and not function_hits and not unit_hits and not dwarf_hits:
        print(f"No symbol matches found for '{args.query}'.")
        return 1

    if symbol_hits:
        print("Symbols:")
        for symbol in symbol_hits[:12]:
            scope = symbol.scope or "-"
            symbol_type = symbol.symbol_type or "-"
            print(
                f"  {symbol.name}  {symbol.section}  {format_hex(symbol.address)}  "
                f"size {format_size(symbol.size)}  {symbol_type}  {scope}"
            )

    if function_hits:
        print("Report functions:")
        for unit, function in function_hits[:12]:
            print(
                f"  {function.name}  {format_hex(function.address)}  size {format_size(function.size)}  "
                f"match {format_percent(function.fuzzy_match_percent)}  {unit.normalized_name}"
            )

    if unit_hits:
        print("Owning units:")
        for unit in unit_hits[:8]:
            source = unit.source_path or "-"
            print(f"  {unit.normalized_name}  {source}")

    if dwarf_hits:
        print("DWARF hints:")
        for line in dwarf_hits:
            print(f"  {line}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
