#!/usr/bin/env python3
"""MCP scaffold for the crashwoc-decomp read-only repo index.

This is a **dependency-free scaffold**, not a full Model Context Protocol
server. A complete MCP server would speak JSON-RPC over stdio with capability
negotiation and typically depend on the `mcp` package. To keep this repo
stdlib-only, this module instead:

1. Documents the intended **read-only** tool surface (``TOOLS`` below) and maps
   each tool to an existing ``tools/ai_*.py`` helper.
2. Dispatches those tools to the helper scripts with ``--json`` so the mapping
   can be tested today (``python tools/mcp_decomp.py call ...``).
3. Provides a minimal newline-delimited JSON stdio bridge (``serve``) for local
   experimentation. This is NOT the MCP wire protocol.

Read-only tools (mapped to existing scripts):

    lookup_symbol(query, version)      -> tools/ai_lookup_symbol.py
    lookup_unit(query, version)        -> tools/ai_lookup_unit.py
    context(query, version)            -> tools/ai_context.py
    match_plan(query, version)         -> tools/ai_match_plan.py
    decompme_inspect(path, version)    -> tools/ai_decompme_zip.py

Deferred / unavailable tools (Ghidra tooling is not present in this repo):

    ghidra_find_function(query, ...)
    ghidra_decompile(address, ...)

Intentionally NOT exposed (would require explicit, non-interactive gating before
ever being added): build_object(unit), build_ctx(unit), changes(), and any form
of arbitrary shell execution. This scaffold is read-only.

To implement a real MCP server later, wire each entry in ``TOOLS`` to an MCP
tool definition and route ``call_tool`` through the JSON-RPC dispatch loop.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from dataclasses import dataclass
from typing import Any, Optional

from ai_common import DEFAULT_VERSION, ROOT


class McpError(Exception):
    """User-facing error from the scaffold."""


@dataclass
class ToolSpec:
    name: str
    summary: str
    script: Optional[str]
    param: str
    supports_version: bool = True
    available: bool = True
    note: str = ""


READONLY_TOOLS: list[ToolSpec] = [
    ToolSpec("lookup_symbol", "Resolve a symbol by name or address.", "tools/ai_lookup_symbol.py", "query"),
    ToolSpec("lookup_unit", "Show unit metadata, paths, and progress.", "tools/ai_lookup_unit.py", "query"),
    ToolSpec("context", "Combined symbol-or-unit context snapshot.", "tools/ai_context.py", "query"),
    ToolSpec("match_plan", "Ranked open functions and next target for a unit.", "tools/ai_match_plan.py", "query"),
    ToolSpec("decompme_inspect", "Inspect a decomp.me zip/dir and resolve placement.", "tools/ai_decompme_zip.py", "path"),
]

DEFERRED_TOOLS: list[ToolSpec] = [
    ToolSpec(
        "ghidra_find_function",
        "Search Ghidra functions by name.",
        None,
        "query",
        available=False,
        note="Ghidra tooling is not present in this repo.",
    ),
    ToolSpec(
        "ghidra_decompile",
        "Decompile a function at an address via Ghidra.",
        None,
        "address",
        available=False,
        note="Ghidra tooling is not present in this repo.",
    ),
]

TOOLS: dict[str, ToolSpec] = {spec.name: spec for spec in (*READONLY_TOOLS, *DEFERRED_TOOLS)}


def tools_payload() -> list[dict[str, Any]]:
    return [
        {
            "name": spec.name,
            "summary": spec.summary,
            "maps_to": spec.script,
            "param": spec.param,
            "supports_version": spec.supports_version,
            "available": spec.available,
            "note": spec.note,
        }
        for spec in TOOLS.values()
    ]


def call_tool(name: str, value: str, version: str = DEFAULT_VERSION) -> Any:
    spec = TOOLS.get(name)
    if spec is None:
        raise McpError(f"Unknown tool: {name}. Known tools: {', '.join(TOOLS)}")
    if not spec.available or spec.script is None:
        raise McpError(f"Tool '{name}' is not available: {spec.note}")

    argv = [sys.executable, str(ROOT / spec.script), value]
    if spec.supports_version:
        argv += ["--version", version]
    argv += ["--json"]
    result = subprocess.run(argv, cwd=ROOT, capture_output=True, text=True)
    if result.returncode != 0:
        message = result.stderr.strip() or f"{spec.script} exited with code {result.returncode}"
        raise McpError(message)
    return json.loads(result.stdout)


def print_tool_list() -> None:
    print("Read-only tools:")
    for spec in READONLY_TOOLS:
        print(f"  {spec.name:<18} {spec.summary}  -> {spec.script}")
    print("\nDeferred / unavailable tools:")
    for spec in DEFERRED_TOOLS:
        print(f"  {spec.name:<18} {spec.summary}  ({spec.note})")
    print("\nNot exposed (read-only scaffold): build_object, build_ctx, changes, shell execution.")


def serve() -> int:
    """Minimal newline-delimited JSON stdio bridge (NOT the MCP wire protocol).

    Reads one JSON object per line: {"tool": "...", "query": "...", "version": "..."}
    (use "path" instead of "query" for decompme_inspect). Writes one JSON
    response per line: {"ok": true, "result": ...} or {"ok": false, "error": ...}.
    """
    print(
        "mcp_decomp scaffold stdio bridge (JSON lines; not MCP protocol). "
        "Send one request object per line; Ctrl-D to exit.",
        file=sys.stderr,
    )
    for raw_line in sys.stdin:
        line = raw_line.strip()
        if not line:
            continue
        try:
            request = json.loads(line)
            tool = request["tool"]
            value = request.get("query", request.get("path"))
            if value is None:
                raise McpError("Request requires 'query' or 'path'.")
            version = request.get("version", DEFAULT_VERSION)
            result = call_tool(tool, value, version)
            response: dict[str, Any] = {"ok": True, "tool": tool, "result": result}
        except (McpError, KeyError, json.JSONDecodeError) as exc:
            response = {"ok": False, "error": str(exc)}
        sys.stdout.write(json.dumps(response) + "\n")
        sys.stdout.flush()
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="MCP scaffold for the crashwoc-decomp read-only repo index.")
    parser.add_argument("--json", action="store_true", help="Print the tool list as JSON (with no subcommand).")
    sub = parser.add_subparsers(dest="command")

    sub.add_parser("list", help="List the intended tool surface (default).")

    p_call = sub.add_parser("call", help="Invoke a read-only tool (for testing the mapping).")
    p_call.add_argument("tool", choices=list(TOOLS))
    p_call.add_argument("value", help="Query string, or path for decompme_inspect.")
    p_call.add_argument("--version", default=DEFAULT_VERSION)

    sub.add_parser("serve", help="Run the minimal stdio JSON-lines bridge (not MCP protocol).")
    return parser


def main(argv: Optional[list[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    if args.command in (None, "list"):
        if getattr(args, "json", False):
            print(json.dumps(tools_payload(), indent=2))
        else:
            print_tool_list()
        return 0

    if args.command == "call":
        try:
            print(json.dumps(call_tool(args.tool, args.value, args.version), indent=2))
        except McpError as exc:
            print(str(exc), file=sys.stderr)
            return 1
        return 0

    if args.command == "serve":
        return serve()

    parser.error(f"Unknown command: {args.command}")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
