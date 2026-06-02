#!/usr/bin/env python3
"""Read-only MCP server for the crashwoc-decomp repo index.

This is a minimal **Model Context Protocol** server implemented with the stdlib
only (no `mcp` package, no other dependencies). ``serve`` speaks JSON-RPC 2.0
over the stdio transport (newline-delimited JSON on stdin/stdout) and implements
``initialize``, ``tools/list``, ``tools/call``, and ``ping``. Each tool maps to
an existing ``tools/ai_*.py`` helper invoked with ``--json``.

The ``list`` and ``call`` subcommands stay available for inspection and for
testing the tool mapping outside an MCP client.

Read-only tools (mapped to existing scripts):

    lookup_symbol(query, version)      -> tools/ai_lookup_symbol.py
    lookup_unit(query, version)        -> tools/ai_lookup_unit.py
    context(query, version)            -> tools/ai_context.py
    match_plan(query, version)         -> tools/ai_match_plan.py
    decompme_inspect(path, version)    -> tools/ai_decompme_zip.py

Deferred / unavailable tools (Ghidra tooling is not present in this repo; not
advertised via tools/list): ghidra_find_function, ghidra_decompile.

Intentionally NOT exposed (would require explicit, non-interactive gating before
ever being added): build_object(unit), build_ctx(unit), changes(), and any form
of arbitrary shell execution. This server is read-only.

Run it from an MCP client with: ``python tools/mcp_decomp.py serve``.
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


# --------------------------------------------------------------------------- #
# Minimal MCP server (JSON-RPC 2.0 over stdio, stdlib only)
# --------------------------------------------------------------------------- #
PROTOCOL_VERSION = "2025-06-18"
SERVER_INFO = {"name": "crashwoc-decomp", "version": "0.1.0"}


def tool_definitions() -> list[dict[str, Any]]:
    """MCP tool definitions for the advertised read-only tools."""
    definitions: list[dict[str, Any]] = []
    for spec in READONLY_TOOLS:
        properties: dict[str, Any] = {
            spec.param: {"type": "string", "description": spec.summary},
        }
        if spec.supports_version:
            properties["version"] = {
                "type": "string",
                "default": DEFAULT_VERSION,
                "description": "Project version under config/ and build/.",
            }
        definitions.append(
            {
                "name": spec.name,
                "description": spec.summary,
                "inputSchema": {
                    "type": "object",
                    "properties": properties,
                    "required": [spec.param],
                },
            }
        )
    return definitions


def _rpc_result(req_id: Any, result: Any) -> dict[str, Any]:
    return {"jsonrpc": "2.0", "id": req_id, "result": result}


def _rpc_error(req_id: Any, code: int, message: str) -> dict[str, Any]:
    return {"jsonrpc": "2.0", "id": req_id, "error": {"code": code, "message": message}}


def _call_tool_content(name: Optional[str], arguments: dict[str, Any]) -> list[dict[str, str]]:
    spec = TOOLS.get(name) if name else None
    if spec is None:
        raise McpError(f"Unknown tool: {name}. Known tools: {', '.join(TOOLS)}")
    value = arguments.get(spec.param) or arguments.get("query") or arguments.get("path")
    if value is None:
        raise McpError(f"Tool '{name}' requires '{spec.param}'.")
    version = arguments.get("version", DEFAULT_VERSION)
    result = call_tool(name, value, version)
    return [{"type": "text", "text": json.dumps(result, indent=2)}]


def handle_request(message: dict[str, Any]) -> Optional[dict[str, Any]]:
    """Handle one JSON-RPC message; return a response, or None for notifications."""
    req_id = message.get("id")
    method = message.get("method")
    params = message.get("params") or {}

    if method == "initialize":
        return _rpc_result(
            req_id,
            {
                "protocolVersion": params.get("protocolVersion") or PROTOCOL_VERSION,
                "capabilities": {"tools": {}},
                "serverInfo": SERVER_INFO,
            },
        )
    if method in ("notifications/initialized", "initialized"):
        return None
    if method == "ping":
        return _rpc_result(req_id, {})
    if method == "tools/list":
        return _rpc_result(req_id, {"tools": tool_definitions()})
    if method == "tools/call":
        try:
            content = _call_tool_content(params.get("name"), params.get("arguments") or {})
            return _rpc_result(req_id, {"content": content, "isError": False})
        except McpError as exc:
            return _rpc_result(
                req_id, {"content": [{"type": "text", "text": str(exc)}], "isError": True}
            )

    if req_id is None:
        return None  # Unknown notification: ignore.
    return _rpc_error(req_id, -32601, f"Method not found: {method}")


def _emit(obj: dict[str, Any]) -> None:
    sys.stdout.write(json.dumps(obj) + "\n")
    sys.stdout.flush()


def serve() -> int:
    """Run the minimal JSON-RPC 2.0 stdio MCP server (stdlib only)."""
    for raw_line in sys.stdin:
        line = raw_line.strip()
        if not line:
            continue
        try:
            message = json.loads(line)
        except json.JSONDecodeError as exc:
            _emit(_rpc_error(None, -32700, f"Parse error: {exc}"))
            continue
        try:
            response = handle_request(message)
        except Exception as exc:  # never let one bad request kill the server
            response = _rpc_error(message.get("id"), -32603, f"Internal error: {exc}")
        if response is not None:
            _emit(response)
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

    sub.add_parser("serve", help="Run the JSON-RPC 2.0 stdio MCP server.")
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
