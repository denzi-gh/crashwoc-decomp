#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from collections import defaultdict
from typing import Any

from ai_common import (
    DEFAULT_VERSION,
    find_units,
    format_percent,
    load_repo_index,
    progress_category_labels,
)


def safe_int(value: Any) -> int:
    if value is None:
        return 0
    if isinstance(value, int):
        return value
    return int(str(value))


def safe_float(value: Any) -> float | None:
    if value is None:
        return None
    return float(value)


def unit_status_payload(unit, category_names: dict[str, str]) -> dict[str, Any]:
    measures = unit.measures
    return {
        "unit": unit.normalized_name,
        "source_path": unit.source_path,
        "report_unit_name": unit.report_unit_name,
        "progress_categories": progress_category_labels(unit, category_names),
        "matched_code_percent": safe_float(measures.get("matched_code_percent")),
        "matched_functions_percent": safe_float(measures.get("matched_functions_percent")),
        "matched_functions": safe_int(measures.get("matched_functions")),
        "total_functions": safe_int(measures.get("total_functions")),
        "matched_code": safe_int(measures.get("matched_code")),
        "total_code": safe_int(measures.get("total_code")),
        "matched_data": safe_int(measures.get("matched_data")),
        "total_data": safe_int(measures.get("total_data")),
    }


def aggregate_categories(index) -> list[dict[str, Any]]:
    buckets: dict[str, dict[str, Any]] = defaultdict(
        lambda: {
            "id": "",
            "name": "",
            "units": 0,
            "matched_functions": 0,
            "total_functions": 0,
            "matched_code": 0,
            "total_code": 0,
            "matched_data": 0,
            "total_data": 0,
        }
    )

    for unit in index.units.values():
        if not unit.measures:
            continue
        category_id = unit.progress_categories[0] if unit.progress_categories else "uncategorized"
        bucket = buckets[category_id]
        bucket["id"] = category_id
        bucket["name"] = index.category_names.get(category_id, category_id)
        bucket["units"] += 1
        bucket["matched_functions"] += safe_int(unit.measures.get("matched_functions"))
        bucket["total_functions"] += safe_int(unit.measures.get("total_functions"))
        bucket["matched_code"] += safe_int(unit.measures.get("matched_code"))
        bucket["total_code"] += safe_int(unit.measures.get("total_code"))
        bucket["matched_data"] += safe_int(unit.measures.get("matched_data"))
        bucket["total_data"] += safe_int(unit.measures.get("total_data"))

    results = list(buckets.values())
    for item in results:
        if item["total_functions"]:
            item["matched_functions_percent"] = item["matched_functions"] / item["total_functions"] * 100.0
        else:
            item["matched_functions_percent"] = None
        if item["total_code"]:
            item["matched_code_percent"] = item["matched_code"] / item["total_code"] * 100.0
        else:
            item["matched_code_percent"] = None
        if item["total_data"]:
            item["matched_data_percent"] = item["matched_data"] / item["total_data"] * 100.0
        else:
            item["matched_data_percent"] = None

    results.sort(key=lambda item: item["id"])
    return results


def print_category_summary(items: list[dict[str, Any]]) -> None:
    print("Category status:")
    for item in items:
        print(
            f"  {item['id']:<16} {format_percent(item['matched_functions_percent']):>8} functions  "
            f"{item['matched_functions']}/{item['total_functions']}  "
            f"code {format_percent(item['matched_code_percent'])}  "
            f"units {item['units']}"
        )


def print_units(items: list[dict[str, Any]]) -> None:
    print("Units:")
    for item in items:
        print(
            f"  {item['unit']:<24} "
            f"functions {format_percent(item['matched_functions_percent']):>8}  "
            f"({item['matched_functions']}/{item['total_functions']})  "
            f"code {format_percent(item['matched_code_percent'])}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Summarize project, category, or unit matching status from build/GCBE7D/report.json."
    )
    parser.add_argument(
        "--version",
        default=DEFAULT_VERSION,
        help=f"Project version under config/ and build/ (default: {DEFAULT_VERSION})",
    )
    parser.add_argument(
        "--category",
        default=None,
        help="Filter output to a specific progress category id such as gameplay.",
    )
    parser.add_argument(
        "--unit",
        default=None,
        help="Filter output to one unit name or source path.",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Emit machine-readable JSON.",
    )
    args = parser.parse_args()

    index = load_repo_index(args.version)
    categories = aggregate_categories(index)

    if args.unit:
        unit_matches = find_units(args.unit, version=args.version)
        if not unit_matches:
            print(f"No unit matches found for '{args.unit}'.")
            return 1
        units_payload = [
            unit_status_payload(unit, index.category_names)
            for unit in unit_matches
            if not args.category or args.category in unit.progress_categories
        ]
        if not units_payload:
            print(f"No units match the requested filters for '{args.unit}'.")
            return 1
        if args.json:
            print(json.dumps(units_payload, indent=2))
            return 0
        print_units(units_payload)
        return 0

    if args.category:
        categories = [item for item in categories if item["id"] == args.category]
        if not categories:
            print(f"No status data found for category '{args.category}'.")
            return 1

    if args.json:
        print(json.dumps(categories, indent=2))
        return 0

    print_category_summary(categories)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
