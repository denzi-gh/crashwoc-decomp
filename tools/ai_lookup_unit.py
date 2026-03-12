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
    load_repo_index,
    progress_category_labels,
    suggest_unit_commands,
    unit_summary,
)


def build_unit_payload(unit, category_names: dict[str, str]) -> dict[str, Any]:
    payload = unit_summary(unit, category_names)
    payload["commands"] = suggest_unit_commands(unit)
    return payload


def print_unit(unit, category_names: dict[str, str]) -> None:
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
    if unit.linked is not None:
        print(f"Linked: {unit.linked}")
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

    print("Commands:")
    for command in suggest_unit_commands(unit):
        print(f"  {command}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Lookup build and split metadata for a unit.")
    parser.add_argument("query", help="Unit name, source path, output path, or address")
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
        "--json",
        action="store_true",
        help="Emit machine-readable JSON",
    )
    args = parser.parse_args()

    index = load_repo_index(args.version)
    matches = find_units(args.query, version=args.version, exact=args.exact)
    if not matches:
        print(f"No unit matches found for '{args.query}'.")
        return 1

    if args.json:
        payload = [build_unit_payload(unit, index.category_names) for unit in matches]
        print(json.dumps(payload, indent=2))
        return 0

    if len(matches) == 1:
        print_unit(matches[0], index.category_names)
        return 0

    print(f"Found {len(matches)} unit matches for '{args.query}':")
    for unit in matches[:12]:
        source = unit.source_path or "-"
        label = unit.build_label or "-"
        print(f"  {unit.normalized_name:<24} {source} [{label}]")
    if len(matches) > 12:
        print(f"  ... and {len(matches) - 12} more")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
