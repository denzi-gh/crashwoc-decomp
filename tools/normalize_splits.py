#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
from pathlib import Path


_ALT_SUFFIX_RE = re.compile(r"^(?P<stem>.+)\.(?P<ext>[A-Za-z0-9]+)_(?P<index>[0-9]+)$")
_NORMALIZED_SUFFIX_RE = re.compile(
    r"^(?P<stem>.+)_(?P<index>[0-9]+)\.(?P<ext>[A-Za-z0-9]+)$"
)
_SPECIAL_NAME_MAP = {
    "GBA.c": "GBA_1.c",
}
_SPECIAL_NAME_MAP_INV = {value: key for key, value in _SPECIAL_NAME_MAP.items()}


def normalize_unit_name(name: str) -> str:
    special = _SPECIAL_NAME_MAP.get(name)
    if special is not None:
        return special
    match = _ALT_SUFFIX_RE.match(name)
    if match is None:
        return name
    return f"{match.group('stem')}_{match.group('index')}.{match.group('ext')}"


def _split_header(line: str) -> tuple[str, str] | None:
    if not line or line.startswith((" ", "\t")) or line == "Sections:" or ":" not in line:
        return None
    name, rest = line.split(":", 1)
    return name, rest


def denormalize_unit_name(name: str, inverse_map: dict[str, str]) -> str:
    special = _SPECIAL_NAME_MAP_INV.get(name)
    if special is not None:
        return special
    mapped = inverse_map.get(name)
    if mapped is not None:
        return mapped

    match = _NORMALIZED_SUFFIX_RE.match(name)
    if match is None:
        return name
    return f"{match.group('stem')}.{match.group('ext')}_{match.group('index')}"


def normalize_splits_text(text: str) -> str:
    out_lines: list[str] = []
    seen_raw: set[str] = set()
    seen_normalized: dict[str, str] = {}

    for line in text.splitlines():
        header = _split_header(line)
        if header is None:
            out_lines.append(line)
            continue

        name, rest = header
        normalized = normalize_unit_name(name)

        if name in seen_raw:
            raise ValueError(f"Duplicate split entry: {name}")
        seen_raw.add(name)

        previous = seen_normalized.get(normalized)
        if previous is not None and previous != name:
            raise ValueError(
                f"Normalized split name collision: {previous} and {name} both map to {normalized}"
            )
        seen_normalized[normalized] = name
        out_lines.append(f"{normalized}:{rest}")

    out_text = "\n".join(out_lines)
    if text.endswith("\n"):
        out_text += "\n"
    return out_text


def _write_if_different(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_file() and path.read_text(encoding="utf-8") == text:
        return
    path.write_text(text, encoding="utf-8")


def write_normalized_splits(input_path: Path, output_path: Path) -> None:
    text = input_path.read_text(encoding="utf-8")
    normalized = normalize_splits_text(text)
    _write_if_different(output_path, normalized)


def write_generated_config(
    config_in_path: Path, config_out_path: Path, generated_splits_path: Path
) -> None:
    config_text = config_in_path.read_text(encoding="utf-8")
    lines = config_text.splitlines()
    replaced = False
    split_line = f"splits: {generated_splits_path.as_posix()}"
    out_lines: list[str] = []

    for line in lines:
        if line.startswith("splits:"):
            out_lines.append(split_line)
            replaced = True
        else:
            out_lines.append(line)

    if not replaced:
        raise ValueError(f"No splits entry found in {config_in_path.as_posix()}")

    out_text = "\n".join(out_lines)
    if config_text.endswith("\n"):
        out_text += "\n"

    _write_if_different(config_out_path, out_text)


def sync_generated_splits_back(source_splits_path: Path, generated_splits_path: Path) -> None:
    if not source_splits_path.exists() or not generated_splits_path.exists():
        return

    source_text = source_splits_path.read_text(encoding="utf-8")
    generated_text = generated_splits_path.read_text(encoding="utf-8")

    inverse_map: dict[str, str] = {}
    for line in source_text.splitlines():
        header = _split_header(line)
        if header is None:
            continue
        raw_name, _ = header
        inverse_map[normalize_unit_name(raw_name)] = raw_name

    out_lines: list[str] = []
    for line in generated_text.splitlines():
        header = _split_header(line)
        if header is None:
            out_lines.append(line)
            continue
        name, rest = header
        out_lines.append(f"{denormalize_unit_name(name, inverse_map)}:{rest}")

    out_text = "\n".join(out_lines)
    if generated_text.endswith("\n"):
        out_text += "\n"
    _write_if_different(source_splits_path, out_text)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate tool-safe dtk split/config files.")
    parser.add_argument("--splits-in", type=Path, required=True)
    parser.add_argument("--splits-out", type=Path, required=True)
    parser.add_argument("--config-in", type=Path, required=True)
    parser.add_argument("--config-out", type=Path, required=True)
    parser.add_argument("--sync-back", action="store_true")
    args = parser.parse_args()

    if args.sync_back:
        sync_generated_splits_back(args.splits_in, args.splits_out)
        return 0

    write_normalized_splits(args.splits_in, args.splits_out)
    write_generated_config(args.config_in, args.config_out, args.splits_out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
