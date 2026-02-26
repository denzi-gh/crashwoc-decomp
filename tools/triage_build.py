#!/usr/bin/env python3
"""
Build triage helper for large decomp bring-up.

This runs compile object targets one-by-one and summarizes the first hard
failure cause, so header/type collisions are easier to fix incrementally.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple


TARGET_RE = re.compile(r"^(?P<target>.+?): (?P<rule>[A-Za-z0-9_.-]+)$")

# Ordered by importance for C header graph issues.
ISSUE_PATTERNS: Sequence[Tuple[re.Pattern[str], str]] = (
    (re.compile(r"redeclared as different kind of symbol", re.IGNORECASE), "symbol-kind clash"),
    (re.compile(r"conflicting types for", re.IGNORECASE), "conflicting declaration"),
    (re.compile(r"redefinition of", re.IGNORECASE), "duplicate definition"),
    (re.compile(r"has incomplete type", re.IGNORECASE), "incomplete type"),
    (re.compile(r"implicit declaration of", re.IGNORECASE), "missing prototype/include"),
    (re.compile(r"undefined reference", re.IGNORECASE), "missing symbol at link"),
    (re.compile(r"fatal", re.IGNORECASE), "fatal compiler error"),
    (re.compile(r"No such file", re.IGNORECASE), "missing include/file"),
)


def run_cmd(args: Sequence[str], cwd: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=str(cwd),
        text=True,
        capture_output=True,
        shell=False,
        check=False,
    )


def discover_compile_targets(root: Path, rules: set[str]) -> List[str]:
    result = run_cmd(["ninja", "-t", "targets", "all"], root)
    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        sys.stderr.write(result.stderr)
        raise RuntimeError("Failed to query ninja targets.")

    out: List[str] = []
    for raw in result.stdout.splitlines():
        line = raw.strip()
        if not line:
            continue
        m = TARGET_RE.match(line)
        if not m:
            continue
        target = m.group("target")
        rule = m.group("rule")
        if not target.endswith(".o"):
            continue
        if rule in rules:
            out.append(target)
    return out


def normalize_path(path: Path) -> str:
    return str(path).replace("\\", "/").lower()


def load_output_to_source_map(root: Path) -> Dict[str, str]:
    db_path = root / "compile_commands.json"
    if not db_path.is_file():
        return {}

    try:
        data = json.loads(db_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return {}

    mapping: Dict[str, str] = {}
    for entry in data:
        output = entry.get("output")
        source = entry.get("file")
        if not output or not source:
            continue
        output_key = normalize_path(Path(output))
        try:
            source_rel = Path(source).resolve().relative_to(root.resolve())
            source_text = source_rel.as_posix()
        except ValueError:
            source_text = source.replace("\\", "/")
        mapping[output_key] = source_text
    return mapping


def infer_source(target: str, root: Path, out_to_src: Dict[str, str]) -> Optional[str]:
    target_key_abs = normalize_path((root / target).resolve())
    if target_key_abs in out_to_src:
        return out_to_src[target_key_abs]

    target_key_rel = normalize_path(Path(target))
    for out_key, src in out_to_src.items():
        if out_key.endswith(target_key_rel):
            return src

    # Fallback guess for dtk-template paths.
    parts = Path(target).parts
    if "src" in parts:
        src_index = parts.index("src")
        rel_src = Path(*parts[src_index:]).with_suffix(".c")
        return rel_src.as_posix()
    return None


def find_primary_issue(log: str) -> Tuple[str, Optional[str]]:
    lines = [line for line in log.splitlines() if line.strip()]
    if not lines:
        return ("no compiler output", None)

    for pat, label in ISSUE_PATTERNS:
        for line in lines:
            if pat.search(line):
                return (line.strip(), label)

    # fallback: last non-empty line
    return (lines[-1].strip(), None)


def print_log_tail(log: str, max_lines: int) -> None:
    lines = [line.rstrip() for line in log.splitlines() if line.strip()]
    if not lines:
        return
    tail = lines[-max_lines:]
    print("  log tail:")
    for line in tail:
        print(f"    {line}")


def should_keep(target: str, source: Optional[str], filters: Iterable[str]) -> bool:
    if not filters:
        return True
    haystack = f"{target} {source or ''}".lower()
    return all(f.lower() in haystack for f in filters)


def main() -> int:
    parser = argparse.ArgumentParser(description="Build object triage helper.")
    parser.add_argument(
        "--root",
        type=Path,
        default=Path("."),
        help="Project root (default: current directory)",
    )
    parser.add_argument(
        "--rules",
        default="prodg_cc,mwcc,cc,cxx",
        help="Comma-separated ninja rule names treated as compile targets",
    )
    parser.add_argument(
        "--match",
        action="append",
        default=[],
        help="Filter by substring (can be used multiple times)",
    )
    parser.add_argument(
        "--all-failures",
        action="store_true",
        help="Continue after failures and report all failing objects",
    )
    parser.add_argument(
        "--list",
        action="store_true",
        help="Only list discovered compile targets",
    )
    parser.add_argument(
        "--log-tail-lines",
        type=int,
        default=8,
        help="How many trailing log lines to print for failures (default: 8)",
    )
    args = parser.parse_args()

    root = args.root.resolve()
    rules = {item.strip() for item in args.rules.split(",") if item.strip()}

    targets = discover_compile_targets(root, rules)
    if not targets:
        print("No compile object targets found.")
        return 1

    out_to_src = load_output_to_source_map(root)
    selected: List[Tuple[str, Optional[str]]] = []
    for target in targets:
        src = infer_source(target, root, out_to_src)
        if should_keep(target, src, args.match):
            selected.append((target, src))

    if not selected:
        print("No targets matched filters.")
        return 1

    if args.list:
        print(f"Found {len(selected)} compile targets:")
        for target, src in selected:
            if src:
                print(f"  {target}  <=  {src}")
            else:
                print(f"  {target}")
        return 0

    print(f"Running triage for {len(selected)} compile targets...")
    failures: List[Tuple[str, Optional[str], str, Optional[str], str]] = []

    for idx, (target, src) in enumerate(selected, start=1):
        label = f"[{idx}/{len(selected)}] {target}"
        print(f"{label} ...", end=" ", flush=True)

        result = run_cmd(["ninja", "-v", target], root)
        if result.returncode == 0:
            print("ok")
            continue

        print("FAIL")
        combined = (result.stdout or "") + ("\n" + result.stderr if result.stderr else "")
        issue_line, issue_kind = find_primary_issue(combined)
        failures.append((target, src, issue_line, issue_kind, combined))

        print(f"  issue: {issue_line}")
        if issue_kind:
            print(f"  class: {issue_kind}")
        if src:
            print(f"  source: {src}")
        print_log_tail(combined, args.log_tail_lines)

        if not args.all_failures:
            print("")
            print("Stopped at first failure. Re-run with --all-failures to continue.")
            return 1

    print("")
    if failures:
        print(f"Done. {len(failures)} target(s) failed.")
        print("Failure summary:")
        for target, src, issue_line, issue_kind, _ in failures:
            kind = f"[{issue_kind}] " if issue_kind else ""
            where = f" ({src})" if src else ""
            print(f"  - {target}{where}: {kind}{issue_line}")
        return 1

    print("Done. All selected targets compiled successfully.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
