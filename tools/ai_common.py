#!/usr/bin/env python3
from __future__ import annotations

import json
import re
from dataclasses import asdict, dataclass, field
from functools import lru_cache
from pathlib import Path
from typing import Any, Iterable, Optional

try:
    from normalize_splits import normalize_unit_name
except ImportError:  # pragma: no cover - import path depends on invocation
    from tools.normalize_splits import normalize_unit_name


ROOT = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "GCBE7D"

SYMBOL_RE = re.compile(
    r"^(?P<name>[^=]+?) = (?P<section>\.[A-Za-z0-9_]+):0x(?P<address>[0-9A-Fa-f]+);"
    r"(?:\s*//\s*(?P<comment>.*))?$"
)
SPLIT_RANGE_RE = re.compile(
    r"^\s*(?P<section>\.[A-Za-z0-9_]+)\s+start:0x(?P<start>[0-9A-Fa-f]+)\s+end:0x(?P<end>[0-9A-Fa-f]+)\s*$"
)
BUILD_COMMENT_RE = re.compile(
    r"^# (?P<unit>[^:]+): (?P<label>.+?) \(linked (?P<linked>True|False)\)$"
)
BUILD_OBJ_RE = re.compile(
    r"^build (?P<output>\S+\.o): (?P<rule>\S+) (?P<source>src[\\/]\S+)"
)
BUILD_CTX_RE = re.compile(
    r"^build (?P<output>\S+\.ctx): decompctx (?P<source>src[\\/]\S+)"
)
ADDRESS_QUERY_RE = re.compile(r"^(?:0x)?(?P<value>[0-9A-Fa-f]{6,8})$")


@dataclass
class SymbolEntry:
    name: str
    section: str
    address: int
    size: Optional[int] = None
    scope: Optional[str] = None
    symbol_type: Optional[str] = None
    comment: str = ""


@dataclass
class SplitRange:
    section: str
    start: int
    end: int

    @property
    def size(self) -> int:
        return self.end - self.start


@dataclass
class ReportFunction:
    name: str
    address: int
    size: int
    fuzzy_match_percent: Optional[float] = None


@dataclass
class UnitInfo:
    raw_name: str
    normalized_name: str
    source_path: Optional[str] = None
    object_path: Optional[str] = None
    ctx_path: Optional[str] = None
    build_label: Optional[str] = None
    linked: Optional[bool] = None
    report_unit_name: Optional[str] = None
    progress_categories: list[str] = field(default_factory=list)
    measures: dict[str, Any] = field(default_factory=dict)
    sections: list[SplitRange] = field(default_factory=list)
    functions: list[ReportFunction] = field(default_factory=list)


@dataclass
class RepoIndex:
    units: dict[str, UnitInfo]
    symbols: list[SymbolEntry]
    category_names: dict[str, str]


def repo_path(*parts: str) -> Path:
    return ROOT.joinpath(*parts)


def rel_posix(path: str | Path) -> str:
    path_text = str(path).replace("\\", "/")
    path_obj = Path(path_text)
    if not path_obj.is_absolute():
        path_obj = ROOT / path_obj
    try:
        rel = path_obj.resolve().relative_to(ROOT.resolve())
        return rel.as_posix()
    except ValueError:
        return path_obj.as_posix()


def parse_address(text: str) -> Optional[int]:
    match = ADDRESS_QUERY_RE.match(text.strip())
    if match is None:
        return None
    return int(match.group("value"), 16)


def format_hex(value: Optional[int]) -> str:
    if value is None:
        return "-"
    return f"0x{value:08X}"


def format_size(value: Optional[int]) -> str:
    if value is None:
        return "-"
    return f"0x{value:X}"


def format_percent(value: Optional[float]) -> str:
    if value is None:
        return "-"
    return f"{value:.2f}%"


def unit_key_from_stem_and_suffix(stem: str, suffix: str) -> str:
    return f"{stem}{suffix.lower()}"


def unit_key_from_source_and_output(source_path: str, output_path: str) -> str:
    return unit_key_from_stem_and_suffix(Path(output_path).stem, Path(source_path).suffix)


def unit_key_from_source_path(source_path: str) -> str:
    return normalize_unit_name(Path(source_path).name)


def unit_summary(unit: UnitInfo, category_names: dict[str, str]) -> dict[str, Any]:
    progress = []
    for category_id in unit.progress_categories:
        progress.append(
            {
                "id": category_id,
                "name": category_names.get(category_id, category_id),
            }
        )
    return {
        "raw_name": unit.raw_name,
        "normalized_name": unit.normalized_name,
        "source_path": unit.source_path,
        "object_path": unit.object_path,
        "ctx_path": unit.ctx_path,
        "build_label": unit.build_label,
        "linked": unit.linked,
        "report_unit_name": unit.report_unit_name,
        "progress_categories": progress,
        "measures": unit.measures,
        "sections": [asdict(section) for section in unit.sections],
        "functions": [asdict(function) for function in unit.functions],
    }


def _ensure_unit(units: dict[str, UnitInfo], key: str, raw_name: Optional[str] = None) -> UnitInfo:
    unit = units.get(key)
    if unit is None:
        raw_value = raw_name or key
        unit = UnitInfo(raw_name=raw_value, normalized_name=key)
        units[key] = unit
    elif raw_name and unit.raw_name == unit.normalized_name:
        unit.raw_name = raw_name
    return unit


def _find_unit_by_source_path(units: dict[str, UnitInfo], source_path: str) -> Optional[UnitInfo]:
    for unit in units.values():
        if unit.source_path == source_path:
            return unit
    return None


def _parse_symbol_comment(comment: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for token in comment.split():
        if ":" not in token:
            continue
        key, value = token.split(":", 1)
        out[key] = value
    return out


@lru_cache(maxsize=1)
def load_repo_index(version: str = DEFAULT_VERSION) -> RepoIndex:
    units: dict[str, UnitInfo] = {}
    category_names: dict[str, str] = {}

    splits_path = repo_path("config", version, "splits.txt")
    if splits_path.is_file():
        current_unit: Optional[UnitInfo] = None
        for raw_line in splits_path.read_text(encoding="utf-8").splitlines():
            line = raw_line.rstrip()
            if not line or line == "Sections:":
                continue
            if not line.startswith("\t") and line.endswith(":"):
                raw_name = line[:-1]
                key = normalize_unit_name(raw_name)
                current_unit = _ensure_unit(units, key, raw_name)
                continue
            if current_unit is None:
                continue
            match = SPLIT_RANGE_RE.match(line)
            if match is None:
                continue
            current_unit.sections.append(
                SplitRange(
                    section=match.group("section"),
                    start=int(match.group("start"), 16),
                    end=int(match.group("end"), 16),
                )
            )

    build_ninja_path = repo_path("build.ninja")
    if build_ninja_path.is_file():
        pending_comment: Optional[dict[str, Any]] = None
        for raw_line in build_ninja_path.read_text(encoding="utf-8").splitlines():
            comment_match = BUILD_COMMENT_RE.match(raw_line)
            if comment_match is not None:
                pending_comment = {
                    "raw_name": comment_match.group("unit"),
                    "build_label": comment_match.group("label"),
                    "linked": comment_match.group("linked") == "True",
                }
                continue

            obj_match = BUILD_OBJ_RE.match(raw_line)
            if obj_match is not None:
                source_path = rel_posix(obj_match.group("source"))
                output_path = rel_posix(obj_match.group("output"))
                key = unit_key_from_source_and_output(source_path, output_path)
                unit = _ensure_unit(
                    units,
                    key,
                    pending_comment["raw_name"] if pending_comment else f"{Path(output_path).stem}{Path(source_path).suffix}",
                )
                unit.source_path = source_path
                unit.object_path = output_path
                if pending_comment is not None:
                    unit.build_label = pending_comment["build_label"]
                    unit.linked = pending_comment["linked"]
                pending_comment = None
                continue

            ctx_match = BUILD_CTX_RE.match(raw_line)
            if ctx_match is None:
                continue
            source_path = rel_posix(ctx_match.group("source"))
            output_path = rel_posix(ctx_match.group("output"))
            key = unit_key_from_source_and_output(source_path, output_path)
            unit = _ensure_unit(units, key, f"{Path(output_path).stem}{Path(source_path).suffix}")
            unit.source_path = source_path
            unit.ctx_path = output_path

    compile_commands_path = repo_path("compile_commands.json")
    if compile_commands_path.is_file():
        compile_commands = json.loads(compile_commands_path.read_text(encoding="utf-8"))
        for entry in compile_commands:
            source_path = rel_posix(entry["file"])
            output_path = rel_posix(entry["output"])
            key = unit_key_from_source_and_output(source_path, output_path)
            unit = _ensure_unit(units, key, f"{Path(output_path).stem}{Path(source_path).suffix}")
            unit.source_path = source_path
            unit.object_path = output_path

    report_path = repo_path("build", version, "report.json")
    if report_path.is_file():
        report = json.loads(report_path.read_text(encoding="utf-8"))
        for category in report.get("categories", []):
            category_names[category["id"]] = category["name"]

        for report_unit in report.get("units", []):
            metadata = report_unit.get("metadata", {})
            source_path = metadata.get("source_path")
            if not source_path:
                continue

            source_rel = rel_posix(source_path)
            unit = _find_unit_by_source_path(units, source_rel)
            if unit is None:
                key = unit_key_from_source_path(source_rel)
                unit = _ensure_unit(units, key)
            unit.source_path = unit.source_path or source_rel
            unit.report_unit_name = report_unit.get("name")
            unit.progress_categories = list(metadata.get("progress_categories", []))
            unit.measures = dict(report_unit.get("measures", {}))
            unit.functions = []
            for function in report_unit.get("functions", []):
                unit.functions.append(
                    ReportFunction(
                        name=function["name"],
                        address=int(function["metadata"]["virtual_address"]),
                        size=int(function.get("size", 0)),
                        fuzzy_match_percent=function.get("fuzzy_match_percent"),
                    )
                )
            unit.functions.sort(key=lambda item: (item.address, item.name))

    symbols: list[SymbolEntry] = []
    symbols_path = repo_path("config", version, "symbols.txt")
    if symbols_path.is_file():
        for raw_line in symbols_path.read_text(encoding="utf-8").splitlines():
            stripped = raw_line.strip()
            if not stripped or stripped.startswith("#"):
                continue
            match = SYMBOL_RE.match(stripped)
            if match is None:
                continue
            comment = match.group("comment") or ""
            attrs = _parse_symbol_comment(comment)
            size_text = attrs.get("size")
            size = int(size_text[2:], 16) if size_text and size_text.startswith("0x") else None
            symbols.append(
                SymbolEntry(
                    name=match.group("name").strip(),
                    section=match.group("section"),
                    address=int(match.group("address"), 16),
                    size=size,
                    scope=attrs.get("scope"),
                    symbol_type=attrs.get("type"),
                    comment=comment,
                )
            )

    return RepoIndex(units=units, symbols=symbols, category_names=category_names)


def source_scan_candidates() -> dict[str, list[str]]:
    out: dict[str, list[str]] = {}
    src_root = repo_path("src")
    if not src_root.is_dir():
        return out
    for pattern in ("*.c", "*.s", "*.cpp"):
        for source_path in src_root.rglob(pattern):
            rel = source_path.relative_to(ROOT).as_posix()
            key = unit_key_from_source_path(rel)
            out.setdefault(key, []).append(rel)
    return out


def attach_source_scan_fallback(index: RepoIndex) -> None:
    candidates = source_scan_candidates()
    for key, paths in candidates.items():
        if len(paths) != 1:
            continue
        unit = index.units.get(key)
        if unit is None:
            unit = _ensure_unit(index.units, key)
        if unit.source_path is None:
            unit.source_path = paths[0]


def find_units(query: str, version: str = DEFAULT_VERSION, exact: bool = False) -> list[UnitInfo]:
    index = load_repo_index(version)
    attach_source_scan_fallback(index)
    query_text = query.strip()
    address = parse_address(query_text)
    if address is not None:
        return find_units_by_address(address, version)

    query_lower = query_text.lower().replace("\\", "/")
    scored: list[tuple[int, UnitInfo]] = []
    for unit in index.units.values():
        fields = {
            "normalized": unit.normalized_name.lower(),
            "raw": unit.raw_name.lower(),
            "source": (unit.source_path or "").lower(),
            "object": (unit.object_path or "").lower(),
            "ctx": (unit.ctx_path or "").lower(),
            "stem": Path(unit.normalized_name).stem.lower(),
        }

        score = -1
        for field_name, field_value in fields.items():
            if not field_value:
                continue
            if query_lower == field_value:
                score = max(score, 160)
            elif query_lower == field_value.replace("\\", "/"):
                score = max(score, 160)
            elif query_lower == Path(field_value).name:
                score = max(score, 150)
            elif query_lower == fields["stem"]:
                score = max(score, 140)
            elif not exact and query_lower in field_value:
                base = 80 if field_name in {"normalized", "raw", "stem"} else 70
                score = max(score, base)
        if score >= 0:
            scored.append((score, unit))

    scored.sort(key=lambda item: (-item[0], item[1].normalized_name))
    return [unit for _, unit in scored]


def find_units_by_address(address: int, version: str = DEFAULT_VERSION) -> list[UnitInfo]:
    index = load_repo_index(version)
    matches: list[tuple[int, UnitInfo]] = []
    for unit in index.units.values():
        best_score = -1
        for section in unit.sections:
            if section.start <= address < section.end:
                score = 2 if section.section == ".text" else 1
                best_score = max(best_score, score)
        if best_score >= 0:
            matches.append((best_score, unit))

    matches.sort(key=lambda item: (-item[0], item[1].normalized_name))
    return [unit for _, unit in matches]


def find_symbols_by_name(query: str, version: str = DEFAULT_VERSION, exact: bool = False) -> list[SymbolEntry]:
    index = load_repo_index(version)
    query_lower = query.strip().lower()
    scored: list[tuple[int, SymbolEntry]] = []
    for symbol in index.symbols:
        name_lower = symbol.name.lower()
        if query_lower == name_lower:
            score = 100
        elif not exact and query_lower in name_lower:
            score = 70
        else:
            continue
        scored.append((score, symbol))

    scored.sort(key=lambda item: (-item[0], item[1].address, item[1].name))
    return [symbol for _, symbol in scored]


def find_symbols_by_address(address: int, version: str = DEFAULT_VERSION) -> list[SymbolEntry]:
    index = load_repo_index(version)
    exact_matches: list[SymbolEntry] = []
    containing_matches: list[SymbolEntry] = []
    for symbol in index.symbols:
        if symbol.address == address:
            exact_matches.append(symbol)
            continue
        if symbol.size is None:
            continue
        if symbol.address <= address < symbol.address + symbol.size:
            containing_matches.append(symbol)

    exact_matches.sort(key=lambda item: (item.section, item.name))
    containing_matches.sort(key=lambda item: (item.address, item.name))
    return exact_matches + containing_matches


def iter_report_functions(version: str = DEFAULT_VERSION) -> Iterable[tuple[UnitInfo, ReportFunction]]:
    index = load_repo_index(version)
    for unit in index.units.values():
        for function in unit.functions:
            yield unit, function


def find_report_functions_by_name(
    query: str, version: str = DEFAULT_VERSION, exact: bool = False
) -> list[tuple[UnitInfo, ReportFunction]]:
    query_lower = query.strip().lower()
    scored: list[tuple[int, UnitInfo, ReportFunction]] = []
    for unit, function in iter_report_functions(version):
        name_lower = function.name.lower()
        if query_lower == name_lower:
            score = 100
        elif not exact and query_lower in name_lower:
            score = 70
        else:
            continue
        scored.append((score, unit, function))

    scored.sort(key=lambda item: (-item[0], item[2].address, item[2].name))
    return [(unit, function) for _, unit, function in scored]


def find_report_functions_by_address(address: int, version: str = DEFAULT_VERSION) -> list[tuple[UnitInfo, ReportFunction]]:
    exact_matches: list[tuple[UnitInfo, ReportFunction]] = []
    containing_matches: list[tuple[UnitInfo, ReportFunction]] = []
    for unit, function in iter_report_functions(version):
        if function.address == address:
            exact_matches.append((unit, function))
            continue
        if function.address <= address < function.address + function.size:
            containing_matches.append((unit, function))

    exact_matches.sort(key=lambda item: (item[1].address, item[1].name))
    containing_matches.sort(key=lambda item: (item[1].address, item[1].name))
    return exact_matches + containing_matches


def progress_category_labels(unit: UnitInfo, category_names: dict[str, str]) -> list[str]:
    out: list[str] = []
    for category_id in unit.progress_categories:
        label = category_names.get(category_id)
        out.append(f"{category_id} ({label})" if label else category_id)
    return out


def suggest_unit_commands(unit: UnitInfo) -> list[str]:
    commands: list[str] = []
    if unit.object_path:
        commands.append(f"ninja {unit.object_path}")
    if unit.ctx_path:
        commands.append(f"ninja {unit.ctx_path}")
    commands.append(f"python tools/ai_lookup_unit.py {unit.normalized_name}")
    return commands


def suggest_regression_commands() -> list[str]:
    return [
        "ninja changes",
        "python tools/changes_fmt.py build/GCBE7D/report_changes.json",
    ]


def nearby_functions(unit: UnitInfo, address: int, radius: int = 2) -> list[ReportFunction]:
    if not unit.functions:
        return []
    for index, function in enumerate(unit.functions):
        if function.address == address:
            start = max(0, index - radius)
            end = min(len(unit.functions), index + radius + 1)
            return unit.functions[start:end]
    return []


def search_dwarf_lines(query: str, limit: int = 5, version: str = DEFAULT_VERSION) -> list[str]:
    dwarf_path = repo_path("src", "dump_alphaNGCport_DWARF.txt")
    if not dwarf_path.is_file():
        return []

    query_lower = query.lower()
    matches: list[str] = []
    with dwarf_path.open("r", encoding="utf-8", errors="ignore") as handle:
        for raw_line in handle:
            line = raw_line.rstrip()
            if query_lower not in line.lower():
                continue
            matches.append(line)
            if len(matches) >= limit:
                break
    return matches
