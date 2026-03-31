#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parent.parent
GHIDRA_SCRIPT_DIR = ROOT / "tools" / "ghidra_scripts"
MARKER = "COPILOT_JSON:"
QUERY_SCRIPT = "CopilotQuery.java"
PROJECT_ENV_VARS = ("CRASHWOC_GHIDRA_PROJECT", "GHIDRA_PROJECT_PATH")
HOME_ENV_VARS = ("CRASHWOC_GHIDRA_HOME", "GHIDRA_HOME")
PROGRAM_ENV_VARS = ("CRASHWOC_GHIDRA_PROGRAM",)


class GhidraCliError(RuntimeError):
    pass


@dataclass(frozen=True)
class GhidraProjectConfig:
    ghidra_home: Path
    project_location: Path
    project_name: str
    program: str | None = None

    @property
    def analyze_headless(self) -> Path:
        return self.ghidra_home / "support" / "analyzeHeadless.bat"

    @property
    def lock_path(self) -> Path:
        return self.project_location / f"{self.project_name}.lock"


def first_env(names: tuple[str, ...]) -> str | None:
    for name in names:
        value = os.environ.get(name)
        if value:
            return value
    return None


def normalize_ghidra_home_candidate(candidate: Path) -> Path | None:
    if candidate.is_file() and candidate.name.lower() == "analyzeheadless.bat":
        return candidate.parent.parent
    if candidate.is_dir() and (candidate / "support" / "analyzeHeadless.bat").is_file():
        return candidate
    if candidate.is_dir() and candidate.name.lower() == "support":
        bat = candidate / "analyzeHeadless.bat"
        if bat.is_file():
            return candidate.parent
    return None


def resolve_project_location_and_name(project_arg: str | None) -> tuple[Path, str]:
    project_text = project_arg or first_env(PROJECT_ENV_VARS)
    if not project_text:
        raise GhidraCliError(
            "No Ghidra project configured. Pass --project <path-to-project-dir-or-.gpr> "
            "or set CRASHWOC_GHIDRA_PROJECT."
        )

    project_path = Path(project_text).expanduser()
    if not project_path.exists():
        raise GhidraCliError(f"Ghidra project path does not exist: {project_path}")

    if project_path.is_file():
        if project_path.suffix.lower() != ".gpr":
            raise GhidraCliError(
                f"Ghidra project path must be a project directory or .gpr file: {project_path}"
            )
        return project_path.parent, project_path.stem

    project_files = sorted(project_path.glob("*.gpr"))
    if len(project_files) == 1:
        return project_path, project_files[0].stem
    if not project_files:
        raise GhidraCliError(
            f"No .gpr file found under Ghidra project directory: {project_path}"
        )
    project_list = ", ".join(path.name for path in project_files[:6])
    raise GhidraCliError(
        f"Multiple Ghidra projects found under {project_path}; pass the .gpr file explicitly. "
        f"Candidates: {project_list}"
    )


def resolve_ghidra_home(explicit_home: str | None, project_location: Path) -> Path:
    candidates: list[Path] = []
    if explicit_home:
        candidates.append(Path(explicit_home).expanduser())
    env_home = first_env(HOME_ENV_VARS)
    if env_home:
        candidates.append(Path(env_home).expanduser())
    candidates.extend([project_location, project_location.parent, project_location.parent.parent])

    seen: set[str] = set()
    for candidate in candidates:
        normalized = normalize_ghidra_home_candidate(candidate)
        if normalized is None:
            continue
        key = str(normalized.resolve()) if normalized.exists() else str(normalized)
        if key in seen:
            continue
        seen.add(key)
        return normalized

    raise GhidraCliError(
        "Could not locate a Ghidra installation with support\\analyzeHeadless.bat. "
        "Pass --ghidra-home <path> or set GHIDRA_HOME."
    )


def resolve_config(
    project: str | None = None,
    ghidra_home: str | None = None,
    program: str | None = None,
) -> GhidraProjectConfig:
    project_location, project_name = resolve_project_location_and_name(project)
    resolved_home = resolve_ghidra_home(ghidra_home, project_location)
    resolved_program = program or first_env(PROGRAM_ENV_VARS)
    return GhidraProjectConfig(
        ghidra_home=resolved_home,
        project_location=project_location,
        project_name=project_name,
        program=resolved_program,
    )


def parse_marker_lines(output: str) -> list[dict[str, Any]]:
    payloads: list[dict[str, Any]] = []
    decoder = json.JSONDecoder()
    for raw_line in output.splitlines():
        line = raw_line.strip()
        marker_index = line.find(MARKER)
        if marker_index < 0:
            continue
        payload, _ = decoder.raw_decode(line[marker_index + len(MARKER) :])
        payloads.append(payload)
    return payloads


def format_command_tail(command: list[str]) -> str:
    return " ".join(command[:6]) + (" ..." if len(command) > 6 else "")


def run_headless_script(
    config: GhidraProjectConfig,
    script_name: str,
    script_args: list[str],
    process_target: str,
    recursive: bool = False,
    allow_locked: bool = False,
    allow_empty: bool = False,
) -> list[dict[str, Any]]:
    if not GHIDRA_SCRIPT_DIR.is_dir():
        raise GhidraCliError(f"Ghidra script directory is missing: {GHIDRA_SCRIPT_DIR}")
    if config.lock_path.exists() and not allow_locked:
        raise GhidraCliError(
            f"Ghidra project appears locked: {config.lock_path}\n"
            "Close the Ghidra GUI for this project or remove a stale lock file, then retry. "
            "Pass --allow-locked only if you are sure the lock is stale."
        )

    command = [
        str(config.analyze_headless),
        str(config.project_location),
        config.project_name,
        "-process",
        process_target,
        "-readOnly",
        "-noanalysis",
    ]
    if recursive:
        command.append("-recursive")
    command.extend(
        [
            "-scriptPath",
            str(GHIDRA_SCRIPT_DIR),
            "-postScript",
            script_name,
            *script_args,
        ]
    )

    try:
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            cwd=ROOT,
            timeout=300,
        )
    except subprocess.TimeoutExpired as exc:
        raise GhidraCliError(
            f"Ghidra headless command timed out while running {script_name}. "
            "This often means the project is open in the Ghidra GUI or the project lock is stale.\n"
            f"Command: {format_command_tail(command)}"
        ) from exc

    combined_output = "\n".join(part for part in (result.stdout, result.stderr) if part)
    payloads = parse_marker_lines(combined_output)
    if payloads:
        return payloads
    if allow_empty and "REPORT SCRIPT ERROR" not in combined_output and f"Execute script: {script_name}" in combined_output:
        return []
    if result.returncode != 0 and not payloads:
        error_tail = "\n".join(combined_output.splitlines()[-20:])
        raise GhidraCliError(
            f"Ghidra headless command failed ({result.returncode}) while running {script_name}.\n"
            f"Command: {format_command_tail(command)}\n"
            f"{error_tail}"
        )
    if not payloads and allow_empty:
        return []
    if not payloads:
        error_tail = "\n".join(combined_output.splitlines()[-20:])
        raise GhidraCliError(
            f"Ghidra did not return any structured data for {script_name}.\n"
            f"Command: {format_command_tail(command)}\n"
            f"{error_tail}"
        )
    return payloads


def list_programs(
    config: GhidraProjectConfig,
    allow_locked: bool = False,
) -> list[dict[str, Any]]:
    payloads = run_headless_script(
        config,
        QUERY_SCRIPT,
        ["programs"],
        "*",
        recursive=True,
        allow_locked=allow_locked,
    )
    payloads.sort(key=lambda item: (item.get("pathname", ""), item.get("name", "")))
    return payloads


def resolve_program_entry(
    config: GhidraProjectConfig,
    program_query: str | None = None,
    allow_locked: bool = False,
) -> dict[str, Any]:
    programs = list_programs(config, allow_locked=allow_locked)
    query = program_query or config.program
    if query is None:
        if len(programs) == 1:
            return programs[0]
        names = ", ".join(program.get("pathname", program.get("name", "?")) for program in programs[:8])
        raise GhidraCliError(
            "Multiple Ghidra programs are available; pass --program <name-or-path> or set "
            f"CRASHWOC_GHIDRA_PROGRAM. Candidates: {names}"
        )

    query_lower = query.lower().replace("\\", "/")
    exact_matches: list[dict[str, Any]] = []
    partial_matches: list[dict[str, Any]] = []
    for program in programs:
        fields = [
            str(program.get("pathname", "")),
            str(program.get("name", "")),
            Path(str(program.get("pathname", ""))).name,
        ]
        normalized_fields = [field.lower().replace("\\", "/") for field in fields if field]
        if query_lower in normalized_fields:
            exact_matches.append(program)
            continue
        if any(query_lower in field for field in normalized_fields):
            partial_matches.append(program)

    if len(exact_matches) == 1:
        return exact_matches[0]
    if len(exact_matches) > 1:
        names = ", ".join(program.get("pathname", program.get("name", "?")) for program in exact_matches[:8])
        raise GhidraCliError(f"Program query '{query}' matched multiple Ghidra programs: {names}")
    if len(partial_matches) == 1:
        return partial_matches[0]
    if partial_matches:
        names = ", ".join(program.get("pathname", program.get("name", "?")) for program in partial_matches[:8])
        raise GhidraCliError(f"Program query '{query}' matched multiple Ghidra programs: {names}")
    raise GhidraCliError(f"No Ghidra program matches '{query}'.")


def headless_process_target(program_entry: dict[str, Any]) -> str:
    pathname = str(program_entry.get("pathname", "")).replace("\\", "/").lstrip("/")
    if pathname:
        return pathname
    return str(program_entry.get("name", ""))


def find_functions(
    query: str,
    config: GhidraProjectConfig,
    program: str | None = None,
    limit: int = 20,
    allow_locked: bool = False,
) -> list[dict[str, Any]]:
    program_entry = resolve_program_entry(config, program, allow_locked=allow_locked)
    payloads = run_headless_script(
        config,
        QUERY_SCRIPT,
        ["find-function", query, str(limit)],
        headless_process_target(program_entry),
        allow_locked=allow_locked,
        allow_empty=True,
    )
    return payloads


def decompile_function(
    address: str,
    config: GhidraProjectConfig,
    program: str | None = None,
    timeout_seconds: int = 60,
    allow_locked: bool = False,
) -> dict[str, Any]:
    program_entry = resolve_program_entry(config, program, allow_locked=allow_locked)
    payloads = run_headless_script(
        config,
        QUERY_SCRIPT,
        ["decompile", address, str(timeout_seconds)],
        headless_process_target(program_entry),
        allow_locked=allow_locked,
    )
    return payloads[0]


def disassemble_address(
    address: str,
    config: GhidraProjectConfig,
    program: str | None = None,
    count: int = 20,
    allow_locked: bool = False,
) -> dict[str, Any]:
    program_entry = resolve_program_entry(config, program, allow_locked=allow_locked)
    payloads = run_headless_script(
        config,
        QUERY_SCRIPT,
        ["disasm", address, str(count)],
        headless_process_target(program_entry),
        allow_locked=allow_locked,
    )
    return payloads[0]


def get_types(
    query: str,
    config: GhidraProjectConfig,
    program: str | None = None,
    limit: int = 10,
    allow_locked: bool = False,
) -> list[dict[str, Any]]:
    program_entry = resolve_program_entry(config, program, allow_locked=allow_locked)
    payloads = run_headless_script(
        config,
        QUERY_SCRIPT,
        ["type-get", query, str(limit)],
        headless_process_target(program_entry),
        allow_locked=allow_locked,
        allow_empty=True,
    )
    return payloads


def print_programs(programs: list[dict[str, Any]]) -> None:
    print("Programs:")
    for program in programs:
        pathname = program.get("pathname", program.get("name", "?"))
        name = program.get("name", "?")
        language = program.get("language_id", "?")
        compiler = program.get("compiler_spec_id", "?")
        print(f"  {pathname}  [{name}]  {language}  {compiler}")


def print_functions(functions: list[dict[str, Any]]) -> None:
    if not functions:
        print("Functions: none")
        return
    print("Functions:")
    for function in functions:
        full_name = function.get("full_name") or function.get("name", "?")
        address = function.get("entry_point", "?")
        signature = function.get("signature", "")
        if signature:
            print(f"  {address}  {full_name}")
            print(f"    {signature}")
        else:
            print(f"  {address}  {full_name}")


def print_decompile(payload: dict[str, Any]) -> None:
    print(f"Program: {payload.get('program_path', '-')}")
    print(f"Function: {payload.get('full_name') or payload.get('name', '-')}")
    print(f"Address: {payload.get('entry_point', payload.get('address', '-'))}")
    print("")
    print(payload.get("code", ""))


def print_disassembly(payload: dict[str, Any]) -> None:
    print(f"Program: {payload.get('program_path', '-')}")
    print(f"Start: {payload.get('address', '-')}")
    print("Instructions:")
    for instruction in payload.get("instructions", []):
        print(f"  {instruction.get('address', '-'):<12} {instruction.get('text', '')}")


def print_types(types: list[dict[str, Any]]) -> None:
    if not types:
        print("Types: none")
        return
    print("Types:")
    for item in types:
        print(f"  {item.get('path_name', item.get('name', '?'))} [{item.get('kind', '?')}]")
        if item.get("kind") in {"structure", "union"}:
            for component in item.get("components", [])[:20]:
                name = component.get("field_name") or "<anon>"
                print(
                    f"    +0x{component.get('offset', 0):X}  {name:<24} "
                    f"{component.get('type_name', '?')} ({component.get('length', '?')})"
                )
        elif item.get("kind") == "enum":
            for enum_value in item.get("values", [])[:20]:
                print(f"    {enum_value.get('name', '?')} = {enum_value.get('value', '?')}")
        elif item.get("kind") == "typedef":
            print(f"    -> {item.get('base_type', '?')}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Query a local Ghidra project through analyzeHeadless.bat."
    )
    parser.add_argument(
        "--project",
        default=None,
        help="Path to a Ghidra project directory or .gpr file. Defaults to CRASHWOC_GHIDRA_PROJECT.",
    )
    parser.add_argument(
        "--ghidra-home",
        default=None,
        help="Path to the Ghidra install root. Defaults to CRASHWOC_GHIDRA_HOME or GHIDRA_HOME.",
    )
    parser.add_argument(
        "--program",
        default=None,
        help="Program name or project pathname. Defaults to CRASHWOC_GHIDRA_PROGRAM when set.",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Emit machine-readable JSON.",
    )
    parser.add_argument(
        "--allow-locked",
        action="store_true",
        help="Attempt to run even if the project lock file exists.",
    )

    subparsers = parser.add_subparsers(dest="command", required=True)

    subparsers.add_parser("programs", help="List programs in the configured Ghidra project.")

    find_parser = subparsers.add_parser("find-function", help="Search functions by name.")
    find_parser.add_argument("query", help="Function name substring to search for.")
    find_parser.add_argument("--limit", type=int, default=20, help="Maximum matches to return.")

    decompile_parser = subparsers.add_parser("decompile", help="Decompile the function containing an address.")
    decompile_parser.add_argument("address", help="Function address such as 0x800032A0")
    decompile_parser.add_argument(
        "--timeout",
        type=int,
        default=60,
        help="Decompile timeout in seconds inside Ghidra (default: 60)",
    )

    disasm_parser = subparsers.add_parser("disasm", help="Disassemble instructions from an address.")
    disasm_parser.add_argument("address", help="Start address such as 0x800032A0")
    disasm_parser.add_argument(
        "-n",
        "--count",
        type=int,
        default=20,
        help="Maximum number of instructions to print (default: 20)",
    )

    type_parser = subparsers.add_parser("type-get", help="Search the program datatype manager.")
    type_parser.add_argument("query", help="Type name substring to search for.")
    type_parser.add_argument("--limit", type=int, default=10, help="Maximum type matches to return.")
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    try:
        config = resolve_config(
            project=args.project,
            ghidra_home=args.ghidra_home,
            program=args.program,
        )

        if args.command == "programs":
            payload: Any = list_programs(config, allow_locked=args.allow_locked)
        elif args.command == "find-function":
            payload = find_functions(
                args.query,
                config,
                program=args.program,
                limit=args.limit,
                allow_locked=args.allow_locked,
            )
        elif args.command == "decompile":
            payload = decompile_function(
                args.address,
                config,
                program=args.program,
                timeout_seconds=args.timeout,
                allow_locked=args.allow_locked,
            )
        elif args.command == "disasm":
            payload = disassemble_address(
                args.address,
                config,
                program=args.program,
                count=args.count,
                allow_locked=args.allow_locked,
            )
        elif args.command == "type-get":
            payload = get_types(
                args.query,
                config,
                program=args.program,
                limit=args.limit,
                allow_locked=args.allow_locked,
            )
        else:  # pragma: no cover - argparse enforces valid commands
            parser.error(f"Unsupported command: {args.command}")
            return 2
    except GhidraCliError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    if args.json:
        print(json.dumps(payload, indent=2))
        return 0

    if args.command == "programs":
        print_programs(payload)
    elif args.command == "find-function":
        print_functions(payload)
    elif args.command == "decompile":
        print_decompile(payload)
    elif args.command == "disasm":
        print_disassembly(payload)
    elif args.command == "type-get":
        print_types(payload)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
