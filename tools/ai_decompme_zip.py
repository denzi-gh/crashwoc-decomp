#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
import zipfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Optional

from ai_common import (
    DEFAULT_VERSION,
    ROOT,
    find_report_functions_by_name,
    find_symbols_by_name,
    find_units_by_address,
    format_hex,
    format_percent,
    format_size,
    suggest_regression_commands,
)


FUNCTION_DEF_RE = re.compile(
    r"(?ms)^(?!\s*(?:if|else|for|while|switch|return)\b)"
    r"\s*(?:[A-Za-z_][\w\s\*\(\)]*?\s+)?(?P<name>[A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{"
)
IDENT_RE = re.compile(r"[A-Za-z_]\w*")
IGNORED_DECLARATION_PREFIXES = ("lbl_", "gap_")


@dataclass
class BundleData:
    path: Path
    source_kind: str
    entries: list[str]
    metadata: dict[str, Any]
    code_text: str
    ctx_text: Optional[str]
    target_s_text: Optional[str]


@dataclass
class SnippetDeclaration:
    name: str
    kind: str
    statement: str


@dataclass
class PlacementHint:
    status: str
    line: Optional[int]
    message: str
    previous_function: Optional[str]
    next_function: Optional[str]


def read_bundle(path: Path) -> BundleData:
    entries: list[str] = []
    metadata: dict[str, Any] = {}
    code_text: Optional[str] = None
    ctx_text: Optional[str] = None
    target_s_text: Optional[str] = None

    if path.is_dir():
        source_kind = "directory"
        files = sorted(item for item in path.rglob("*") if item.is_file())
        entries = [item.relative_to(path).as_posix() for item in files]
        code_path = find_named_path(files, "code.c")
        if code_path is None:
            raise FileNotFoundError(f"Missing code.c in {path}")
        code_text = code_path.read_text(encoding="utf-8")
        metadata_path = find_named_path(files, "metadata.json")
        if metadata_path is not None:
            metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
        ctx_path = find_named_path(files, "ctx.c")
        if ctx_path is not None:
            ctx_text = ctx_path.read_text(encoding="utf-8", errors="ignore")
        target_s_path = find_named_path(files, "target.s")
        if target_s_path is not None:
            target_s_text = target_s_path.read_text(encoding="utf-8", errors="ignore")
    else:
        source_kind = "zip"
        with zipfile.ZipFile(path) as archive:
            entries = sorted(item.filename for item in archive.infolist() if not item.is_dir())
            code_entry = find_named_zip_entry(entries, "code.c")
            if code_entry is None:
                raise FileNotFoundError(f"Missing code.c in {path}")
            code_text = archive.read(code_entry).decode("utf-8")
            metadata_entry = find_named_zip_entry(entries, "metadata.json")
            if metadata_entry is not None:
                metadata = json.loads(archive.read(metadata_entry).decode("utf-8"))
            ctx_entry = find_named_zip_entry(entries, "ctx.c")
            if ctx_entry is not None:
                ctx_text = archive.read(ctx_entry).decode("utf-8", errors="ignore")
            target_s_entry = find_named_zip_entry(entries, "target.s")
            if target_s_entry is not None:
                target_s_text = archive.read(target_s_entry).decode("utf-8", errors="ignore")

    return BundleData(
        path=path,
        source_kind=source_kind,
        entries=entries,
        metadata=metadata,
        code_text=code_text,
        ctx_text=ctx_text,
        target_s_text=target_s_text,
    )


def iter_function_names(code_text: str) -> list[str]:
    out: list[str] = []
    seen: set[str] = set()
    for match in FUNCTION_DEF_RE.finditer(code_text):
        name = match.group("name")
        if name in seen:
            continue
        seen.add(name)
        out.append(name)
    return out


def find_named_path(paths: list[Path], basename: str) -> Optional[Path]:
    matches = [path for path in paths if path.name == basename]
    if not matches:
        return None
    return matches[0]


def find_named_zip_entry(entries: list[str], basename: str) -> Optional[str]:
    matches = [entry for entry in entries if Path(entry).name == basename]
    if not matches:
        return None
    return matches[0]


def line_number_for_offset(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def named_function_line(text: str, name: str) -> Optional[int]:
    pattern = re.compile(
        rf"(?ms)^(?!\s*(?:if|else|for|while|switch|return)\b)"
        rf"\s*(?:[A-Za-z_][\w\s\*\(\)]*?\s+)?{re.escape(name)}\s*\([^;{{}}]*\)\s*\{{"
    )
    match = pattern.search(text)
    if match is None:
        return None
    return line_number_for_offset(text, match.start())


def split_top_level_statements(text: str) -> list[str]:
    statements: list[str] = []
    start = 0
    brace_depth = 0
    paren_depth = 0
    bracket_depth = 0
    i = 0
    while i < len(text):
        ch = text[i]
        next_two = text[i : i + 2]

        if next_two == "//":
            newline = text.find("\n", i + 2)
            if newline == -1:
                break
            i = newline + 1
            continue
        if next_two == "/*":
            end = text.find("*/", i + 2)
            if end == -1:
                break
            i = end + 2
            continue
        if ch in {"'", '"'}:
            quote = ch
            i += 1
            while i < len(text):
                if text[i] == "\\":
                    i += 2
                    continue
                if text[i] == quote:
                    i += 1
                    break
                i += 1
            continue
        if ch == "{":
            brace_depth += 1
        elif ch == "}":
            brace_depth = max(0, brace_depth - 1)
        elif ch == "(":
            paren_depth += 1
        elif ch == ")":
            paren_depth = max(0, paren_depth - 1)
        elif ch == "[":
            bracket_depth += 1
        elif ch == "]":
            bracket_depth = max(0, bracket_depth - 1)
        elif ch == ";" and brace_depth == 0 and paren_depth == 0 and bracket_depth == 0:
            statement = text[start : i + 1].strip()
            if statement:
                statements.append(statement)
            start = i + 1
        i += 1
    return statements


def extract_function_name_from_declaration(statement: str) -> Optional[str]:
    if "(" not in statement:
        return None
    prefix = statement[: statement.find("(")]
    matches = IDENT_RE.findall(prefix)
    if not matches:
        return None
    return matches[-1]


def extract_variable_name(statement: str) -> Optional[str]:
    trimmed = statement.rstrip(";").strip()
    if not trimmed:
        return None
    if trimmed.startswith("extern "):
        trimmed = trimmed[7:].strip()
    trimmed = trimmed.split("=", 1)[0].strip()
    match = re.search(r"([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*$", trimmed)
    if match is None:
        return None
    return match.group(1)


def collect_snippet_declarations(code_text: str) -> list[SnippetDeclaration]:
    function_matches = list(FUNCTION_DEF_RE.finditer(code_text))
    preamble = code_text[: function_matches[0].start()] if function_matches else code_text
    declarations: list[SnippetDeclaration] = []
    for statement in split_top_level_statements(preamble):
        normalized = " ".join(line.strip() for line in statement.splitlines()).strip()
        if not normalized or normalized.startswith("#"):
            continue

        if normalized.startswith("typedef "):
            continue

        if "(" in normalized and normalized.endswith(";"):
            name = extract_function_name_from_declaration(normalized)
            if name and not is_ignored_declaration_name(name):
                kind = "extern_function" if normalized.startswith("extern ") else "prototype"
                declarations.append(
                    SnippetDeclaration(name=name, kind=kind, statement=normalized)
                )
            continue

        name = extract_variable_name(normalized)
        if name and not is_ignored_declaration_name(name):
            kind = "extern_variable" if normalized.startswith("extern ") else "global"
            declarations.append(SnippetDeclaration(name=name, kind=kind, statement=normalized))
    return declarations


def is_ignored_declaration_name(name: str) -> bool:
    return any(name.startswith(prefix) for prefix in IGNORED_DECLARATION_PREFIXES)


def decl_missing_from_source(declaration: SnippetDeclaration, source_text: str) -> bool:
    return re.search(rf"\b{re.escape(declaration.name)}\b", source_text) is None


def placement_hint_for_function(unit, function_name: str) -> PlacementHint:
    if not unit.source_path:
        return PlacementHint(
            status="unknown",
            line=None,
            message="Unit has no source path in repo index.",
            previous_function=None,
            next_function=None,
        )

    source_path = ROOT / unit.source_path
    if not source_path.is_file():
        return PlacementHint(
            status="unknown",
            line=None,
            message=f"Source file not found: {unit.source_path}",
            previous_function=None,
            next_function=None,
        )

    source_text = source_path.read_text(encoding="utf-8", errors="ignore")
    existing_line = named_function_line(source_text, function_name)
    if existing_line is not None:
        return PlacementHint(
            status="present",
            line=existing_line,
            message=f"Function already exists in source at line {existing_line}.",
            previous_function=None,
            next_function=None,
        )

    if not unit.functions:
        return PlacementHint(
            status="missing",
            line=None,
            message="Function missing from source; no report ordering available for placement hint.",
            previous_function=None,
            next_function=None,
        )

    index = None
    for i, function in enumerate(unit.functions):
        if function.name == function_name:
            index = i
            break

    if index is None:
        return PlacementHint(
            status="missing",
            line=None,
            message="Function missing from source and not present in report ordering.",
            previous_function=None,
            next_function=None,
        )

    for next_function in unit.functions[index + 1 :]:
        line = named_function_line(source_text, next_function.name)
        if line is not None:
            return PlacementHint(
                status="missing",
                line=line,
                message=f"Function missing from source; insert before {next_function.name} at line {line}.",
                previous_function=unit.functions[index - 1].name if index > 0 else None,
                next_function=next_function.name,
            )

    for previous_function in reversed(unit.functions[:index]):
        line = named_function_line(source_text, previous_function.name)
        if line is not None:
            return PlacementHint(
                status="missing",
                line=line,
                message=f"Function missing from source; insert after {previous_function.name} (starts at line {line}).",
                previous_function=previous_function.name,
                next_function=None,
            )

    return PlacementHint(
        status="missing",
        line=None,
        message="Function missing from source; could not anchor placement to nearby source functions.",
        previous_function=unit.functions[index - 1].name if index > 0 else None,
        next_function=unit.functions[index + 1].name if index + 1 < len(unit.functions) else None,
    )


def resolve_function(function_name: str, version: str) -> dict[str, Any]:
    symbol_hits = find_symbols_by_name(function_name, version=version, exact=True)
    report_hits = find_report_functions_by_name(function_name, version=version, exact=True)

    unit = report_hits[0][0] if report_hits else None
    report_function = report_hits[0][1] if report_hits else None
    symbol = symbol_hits[0] if symbol_hits else None

    if unit is None and symbol is not None:
        unit_hits = find_units_by_address(symbol.address, version=version)
        unit = unit_hits[0] if unit_hits else None

    hint = placement_hint_for_function(unit, function_name) if unit is not None else None
    return {
        "name": function_name,
        "symbol": symbol,
        "unit": unit,
        "report_function": report_function,
        "placement_hint": hint,
    }


def iter_commands_for_resolved_functions(resolved_functions: Iterable[dict[str, Any]]) -> list[str]:
    out: list[str] = []
    seen: set[str] = set()
    for resolved in resolved_functions:
        name = resolved["name"]
        unit = resolved["unit"]
        commands = [f"python tools/ai_context.py {name}"]
        if unit is not None:
            if unit.ctx_path:
                commands.append(f"ninja {unit.ctx_path}")
            if unit.object_path:
                commands.append(f"ninja {unit.object_path}")
            commands.append(f"python tools/ai_lookup_unit.py {unit.normalized_name}")
        commands.extend(suggest_regression_commands())
        for command in commands:
            if command in seen:
                continue
            seen.add(command)
            out.append(command)
    return out


def json_ready_symbol(symbol) -> Optional[dict[str, Any]]:
    if symbol is None:
        return None
    return {
        "name": symbol.name,
        "section": symbol.section,
        "address": symbol.address,
        "size": symbol.size,
        "scope": symbol.scope,
        "symbol_type": symbol.symbol_type,
    }


def json_ready_unit(unit) -> Optional[dict[str, Any]]:
    if unit is None:
        return None
    return {
        "name": unit.raw_name,
        "normalized_name": unit.normalized_name,
        "source_path": unit.source_path,
        "object_path": unit.object_path,
        "ctx_path": unit.ctx_path,
        "build_label": unit.build_label,
    }


def json_ready_report_function(function) -> Optional[dict[str, Any]]:
    if function is None:
        return None
    return {
        "name": function.name,
        "address": function.address,
        "size": function.size,
        "fuzzy_match_percent": function.fuzzy_match_percent,
    }


def json_ready_hint(hint: Optional[PlacementHint]) -> Optional[dict[str, Any]]:
    if hint is None:
        return None
    return {
        "status": hint.status,
        "line": hint.line,
        "message": hint.message,
        "previous_function": hint.previous_function,
        "next_function": hint.next_function,
    }


def build_payload(bundle: BundleData, resolved_functions: list[dict[str, Any]]) -> dict[str, Any]:
    unit_sources: dict[str, str] = {}
    for resolved in resolved_functions:
        unit = resolved["unit"]
        if unit is None or not unit.source_path:
            continue
        source_path = ROOT / unit.source_path
        if source_path.is_file():
            unit_sources[unit.normalized_name] = source_path.read_text(
                encoding="utf-8", errors="ignore"
            )

    declarations = collect_snippet_declarations(bundle.code_text)
    missing_by_unit: dict[str, list[dict[str, str]]] = {}
    for resolved in resolved_functions:
        unit = resolved["unit"]
        if unit is None or unit.normalized_name not in unit_sources:
            continue
        missing = [
            {"name": decl.name, "kind": decl.kind, "statement": decl.statement}
            for decl in declarations
            if decl_missing_from_source(decl, unit_sources[unit.normalized_name])
        ]
        missing_by_unit[unit.normalized_name] = missing

    return {
        "bundle": {
            "path": str(bundle.path),
            "source_kind": bundle.source_kind,
            "entries": bundle.entries,
            "metadata": bundle.metadata,
            "function_names": iter_function_names(bundle.code_text),
        },
        "functions": [
            {
                "name": resolved["name"],
                "symbol": json_ready_symbol(resolved["symbol"]),
                "unit": json_ready_unit(resolved["unit"]),
                "report_function": json_ready_report_function(resolved["report_function"]),
                "placement_hint": json_ready_hint(resolved["placement_hint"]),
            }
            for resolved in resolved_functions
        ],
        "snippet_declarations": [
            {"name": decl.name, "kind": decl.kind, "statement": decl.statement}
            for decl in declarations
        ],
        "missing_declarations_by_unit": missing_by_unit,
        "commands": iter_commands_for_resolved_functions(resolved_functions),
    }


def print_bundle_summary(bundle: BundleData, resolved_functions: list[dict[str, Any]]) -> None:
    print(f"Bundle: {bundle.metadata.get('name', bundle.path.stem)}")
    print(f"Path: {bundle.path}")
    print(f"Source kind: {bundle.source_kind}")
    if bundle.metadata.get("slug"):
        print(f"Slug: {bundle.metadata['slug']}")
    if bundle.metadata.get("compiler"):
        print(f"Compiler: {bundle.metadata['compiler']}")
    if bundle.metadata.get("platform"):
        print(f"Platform: {bundle.metadata['platform']}")
    if bundle.entries:
        print(f"Entries: {', '.join(bundle.entries)}")

    function_names = iter_function_names(bundle.code_text)
    if function_names:
        print("Snippet functions:")
        for name in function_names:
            print(f"  {name}")

    if resolved_functions:
        print("Resolved targets:")
        for resolved in resolved_functions:
            symbol = resolved["symbol"]
            unit = resolved["unit"]
            report_function = resolved["report_function"]
            hint = resolved["placement_hint"]

            symbol_text = format_hex(symbol.address) if symbol is not None else "-"
            size_text = format_size(symbol.size) if symbol is not None else "-"
            print(f"  {resolved['name']}")
            print(f"    Symbol: {symbol_text} size {size_text}")
            if report_function is not None:
                print(f"    Match: {format_percent(report_function.fuzzy_match_percent)}")
            if unit is not None:
                print(f"    Unit: {unit.normalized_name}")
                if unit.source_path:
                    print(f"    Source: {unit.source_path}")
                if unit.ctx_path:
                    print(f"    Context: {unit.ctx_path}")
            if hint is not None:
                print(f"    Placement: {hint.message}")

    declarations = collect_snippet_declarations(bundle.code_text)
    if declarations:
        print("Snippet declarations:")
        for decl in declarations[:16]:
            print(f"  {decl.kind:<15} {decl.name}")
        if len(declarations) > 16:
            print(f"  ... and {len(declarations) - 16} more")

    printed_missing = False
    for resolved in resolved_functions:
        unit = resolved["unit"]
        if unit is None or not unit.source_path:
            continue
        source_path = ROOT / unit.source_path
        if not source_path.is_file():
            continue
        source_text = source_path.read_text(encoding="utf-8", errors="ignore")
        missing = [decl for decl in declarations if decl_missing_from_source(decl, source_text)]
        if not missing:
            continue
        if not printed_missing:
            print("Candidate missing snippet declarations:")
            printed_missing = True
        print(f"  {unit.normalized_name}")
        for decl in missing[:16]:
            print(f"    {decl.kind:<15} {decl.name}")
        if len(missing) > 16:
            print(f"    ... and {len(missing) - 16} more")

    commands = iter_commands_for_resolved_functions(resolved_functions)
    if commands:
        print("Commands:")
        for command in commands:
            print(f"  {command}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Inspect a decomp.me-style zip or extracted directory and resolve repo integration hints."
    )
    parser.add_argument("bundle", help="Path to a .zip export or extracted directory")
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

    bundle_path = Path(args.bundle)
    if not bundle_path.exists():
        print(f"Bundle path does not exist: {bundle_path}")
        return 1

    try:
        bundle = read_bundle(bundle_path)
    except (FileNotFoundError, json.JSONDecodeError, zipfile.BadZipFile) as exc:
        print(f"Failed to read bundle: {exc}")
        return 1

    resolved_functions = [resolve_function(name, args.version) for name in iter_function_names(bundle.code_text)]
    if args.json:
        print(json.dumps(build_payload(bundle, resolved_functions), indent=2))
        return 0

    print_bundle_summary(bundle, resolved_functions)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
