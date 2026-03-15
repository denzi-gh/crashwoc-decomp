#!/usr/bin/env python3
"""
Pi-agent orchestrator for parallel autonomous decompilation matching.

Reads report.json to find unmatched functions, launches pi-coding-agent
subagents (one per C file), monitors progress via a live terminal dashboard.

Usage:
    python tools/pi_orchestrator.py [OPTIONS]
    python tools/pi_orchestrator.py --dasboard tmp/pi_orchestrator_summary

Options:
    --max-concurrent N     Parallel agents (default: 4)
    --include PATTERN      Only units matching glob (e.g. src/gamecode/*)
    --exclude PATTERN      Skip units matching glob
    --poll-interval SECS   Progress poll interval (default: 30)
    --stall-timeout MINS   Minutes before declaring stall (default: 30)
    --dry-run              Show work queue without launching
    --no-dashboard         Simple logging instead of live dashboard
    --use-subagent         Use pi-subagent spawn instead of direct pi
"""
from __future__ import annotations

import argparse
import json
import os
import platform
import re
import shutil
import signal
import subprocess
import sys
import time
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from fnmatch import fnmatch
from pathlib import Path
from typing import Optional

# Ensure tools/ is on sys.path for ai_common imports
TOOLS_DIR = Path(__file__).resolve().parent
ROOT = TOOLS_DIR.parent
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

from ai_common import (
    DEFAULT_VERSION,
    load_repo_index,
    find_units,
    next_unmatched_function,
    ranked_unit_functions,
    unit_match_counts,
)
from pi_dashboard import (
    BlockedUnit,
    CompletedUnit,
    Dashboard,
    DashboardState,
    SlotDisplay,
    print_simple_status,
    HAS_RICH,
)
from pi_summary_dashboard import open_summary_dashboard


PROMPT_TEMPLATE = (
    "Arbeite an der {function} in {source_path} bis alle verbleibenden "
    "Funktionen 100% matched sind oder klar blockiert sind. Starte mit "
    "python tools/ai_match_plan.py {source_path}. Arbeite immer nur an einer Funktion "
    "gleichzeitig, benutze nur C-Code und kein inline asm, baue nur das Einzelobjekt "
    "und pruefe nach jeder Aenderung den Fortschritt neu. Denk an die GitHub instructions "
    "im .md files. Frag mich nichts. Ich gehe schlafen. Ich werde auf nichts antworten. "
    "arbeite solange bis alle Funktionen zu 100% matched sind."
)

SYSTEM_PROMPT_PATH = ".pi/agents/decomp-matcher.md"
SUMMARY_DIRNAME = "pi_orchestrator_summary"


@dataclass
class WorkItem:
    source_path: str
    unit_name: str
    remaining: int
    total: int
    first_function: str
    priority_score: float = 0.0


@dataclass
class ActiveSlot:
    slot_id: int
    process: subprocess.Popen
    work_item: WorkItem
    start_time: datetime
    initial_remaining: int
    current_remaining: int
    last_progress_time: datetime
    log_path: str
    last_log_size: int = 0
    log_growing: bool = False


def _reload_repo_index(version: str = DEFAULT_VERSION):
    """Clear lru_cache and reload the repo index for fresh progress data."""
    load_repo_index.cache_clear()
    return load_repo_index(version)


def build_work_queue(
    version: str = DEFAULT_VERSION,
    include_patterns: Optional[list[str]] = None,
    exclude_patterns: Optional[list[str]] = None,
) -> list[WorkItem]:
    """Build a prioritized queue of units with unmatched functions."""
    index = _reload_repo_index(version)
    items: list[WorkItem] = []
    seen_sources: set[str] = set()

    for key, unit in index.units.items():
        if not unit.source_path or not unit.functions:
            continue

        # Deduplicate by source path
        if unit.source_path in seen_sources:
            continue
        seen_sources.add(unit.source_path)

        counts = unit_match_counts(unit)
        if counts["remaining"] <= 0:
            continue

        # Apply include/exclude filters
        if include_patterns:
            if not any(fnmatch(unit.source_path, p) for p in include_patterns):
                continue
        if exclude_patterns:
            if any(fnmatch(unit.source_path, p) for p in exclude_patterns):
                continue

        next_fn = next_unmatched_function(unit)
        fn_name = next_fn.name if next_fn else "unknown"

        # Priority: fewer remaining = higher priority (quick wins first)
        # Tiebreak: higher match ratio = closer to done
        match_ratio = counts["matched"] / counts["total"] if counts["total"] else 0
        avg_partial = 0.0
        unmatched = ranked_unit_functions(unit, include_matched=False)
        if unmatched:
            partials = [f.fuzzy_match_percent for f in unmatched if f.fuzzy_match_percent is not None]
            if partials:
                avg_partial = sum(partials) / len(partials)

        # Lower score = higher priority (we sort ascending)
        score = counts["remaining"] * 10 - match_ratio * 40 - avg_partial * 0.3

        items.append(WorkItem(
            source_path=unit.source_path,
            unit_name=unit.normalized_name,
            remaining=counts["remaining"],
            total=counts["total"],
            first_function=fn_name,
            priority_score=score,
        ))

    items.sort(key=lambda w: w.priority_score)
    return items


def _build_prompt(work_item: WorkItem) -> str:
    return PROMPT_TEMPLATE.format(
        function=work_item.first_function,
        source_path=work_item.source_path,
    )


def launch_subagent(
    work_item: WorkItem,
    slot_id: int,
    provider: str = "github-copilot",
    model: str = "claude-opus-4-6",
    thinking: str = "high",
    log_dir: str = "tmp",
) -> ActiveSlot:
    """Launch a pi-coding-agent process for the given work item."""
    log_name = work_item.unit_name.replace(".", "_").replace("/", "_")
    log_path = os.path.join(log_dir, f"pi_agent_{log_name}.log")
    os.makedirs(log_dir, exist_ok=True)

    prompt = _build_prompt(work_item)

    # Resolve the pi executable — on Windows it's a .cmd wrapper that
    # subprocess.Popen cannot find without shell=True, so we locate it
    # explicitly via shutil.which which checks .cmd/.ps1 extensions.
    pi_exe = shutil.which("pi")
    if pi_exe is None:
        raise FileNotFoundError(
            "Could not find 'pi' on PATH. "
            "Install with: npm install -g @mariozechner/pi-coding-agent"
        )

    system_prompt_abs = str(ROOT / SYSTEM_PROMPT_PATH)
    cmd = [
        pi_exe,
        "-p",                                   # non-interactive print mode
        "--mode", "json",                        # stream JSON events for live output
        "--provider", provider,                  # github-copilot
        "--model", model,                        # claude-opus-4.6
        "--thinking", thinking,                  # high
        "--system-prompt", system_prompt_abs,
        "--no-session",                          # don't save session files
        prompt,
    ]

    log_file = open(log_path, "w", encoding="utf-8")
    is_windows = platform.system() == "Windows"
    process = subprocess.Popen(
        cmd,
        cwd=str(ROOT),
        stdout=log_file,
        stderr=subprocess.STDOUT,
        # shell=True on Windows so .cmd wrappers are handled correctly
        shell=is_windows,
        creationflags=getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0) if is_windows else 0,
    )

    now = datetime.now()
    return ActiveSlot(
        slot_id=slot_id,
        process=process,
        work_item=work_item,
        start_time=now,
        initial_remaining=work_item.remaining,
        current_remaining=work_item.remaining,
        last_progress_time=now,
        log_path=log_path,
    )


def _detect_current_function(log_path: str) -> str:
    """Parse the tail of a pi JSON-mode log to detect which function is being worked on.

    Looks at text_delta content and bash tool calls for function name references.
    """
    try:
        with open(log_path, "r", encoding="utf-8", errors="replace") as f:
            f.seek(0, 2)
            size = f.tell()
            f.seek(max(0, size - 32768))
            tail = f.read()
    except (OSError, IOError):
        return ""

    # Collect all text from JSON events
    all_text = []
    for raw_line in reversed(tail.splitlines()[-300:]):
        raw_line = raw_line.strip()
        if not raw_line.startswith("{"):
            # Plain text fallback
            all_text.append(raw_line)
            continue
        try:
            obj = json.loads(raw_line)
        except (json.JSONDecodeError, ValueError):
            continue
        ev_type = obj.get("type", "")
        if ev_type == "message_update":
            ame = obj.get("assistantMessageEvent", {})
            ame_type = ame.get("type", "")
            if ame_type == "text_delta":
                delta = ame.get("delta", "")
                if delta.strip():
                    all_text.append(delta.strip())
            elif ame_type == "tool_call_start":
                tc = ame.get("toolCall", {})
                args = tc.get("arguments", {})
                # Check bash commands for function references
                cmd = args.get("command", "")
                if cmd:
                    all_text.append(cmd)

    # Search collected text for function names
    patterns = [
        r"Next target:\s+(\w+)",
        r"ai_context\.py\s+(\w+)",
        r"ai_lookup_symbol\.py\s+(\w+)",
        r"0x[0-9A-Fa-f]+\s+(\w{3,})\s+size",
        r"(?:Editing|Working on|Matching|Bearbeite)\s+[`*]*(\w{3,})[`*]*",
        r"^[`*-]*\s*(\w{3,})\s+[✅✗←]",
    ]
    for text in all_text:
        clean = re.sub(r"\x1b\[[0-9;]*[a-zA-Z]", "", text)
        for pattern in patterns:
            m = re.search(pattern, clean)
            if m:
                return m.group(1)

    return ""


def _check_log_activity(slot: ActiveSlot) -> None:
    """Update log_growing and last_log_size by checking the log file."""
    try:
        current_size = os.path.getsize(slot.log_path)
    except OSError:
        current_size = 0
    slot.log_growing = current_size > slot.last_log_size
    slot.last_log_size = current_size


def _detect_agent_error(log_path: str) -> bool:
    """Check if the agent log contains fatal errors (e.g. missing API key, model not supported)."""
    try:
        with open(log_path, "r", encoding="utf-8", errors="replace") as f:
            f.seek(0, 2)
            size = f.tell()
            f.seek(max(0, size - 4096))
            tail = f.read()
        clean = re.sub(r"\x1b\[[0-9;]*[a-zA-Z]|\x1b\][^\x07]*\x07", "", tail)
        error_patterns = [
            "Error: No API key",
            "model is not supported",
            "Error: Authentication",
            "rate limit",
        ]
        for pattern in error_patterns:
            if pattern.lower() in clean.lower():
                return True
    except (OSError, IOError):
        pass
    return False


def poll_progress(
    active_slots: list[ActiveSlot],
    version: str = DEFAULT_VERSION,
) -> None:
    """Re-read report.json and update remaining counts on active slots."""
    index = _reload_repo_index(version)
    now = datetime.now()

    for slot in active_slots:
        # Find the unit by source path
        unit = None
        for u in index.units.values():
            if u.source_path == slot.work_item.source_path:
                unit = u
                break

        if unit is None:
            continue

        counts = unit_match_counts(unit)
        new_remaining = counts["remaining"]

        if new_remaining < slot.current_remaining:
            slot.last_progress_time = now
        slot.current_remaining = new_remaining


def _slot_to_display(slot: ActiveSlot, stall_timeout_mins: int) -> SlotDisplay:
    now = datetime.now()
    elapsed = now - slot.start_time
    stall_duration = now - slot.last_progress_time
    is_stalled = stall_duration.total_seconds() > stall_timeout_mins * 60
    process_alive = slot.process.poll() is None

    # Determine status
    if slot.current_remaining == 0:
        status = "finishing"
    elif not process_alive:
        status = "error"
    elif is_stalled:
        status = "stalled"
    else:
        status = "active"

    # Check for fatal errors in log
    if status == "active" and _detect_agent_error(slot.log_path):
        status = "error"

    current_fn = _detect_current_function(slot.log_path)

    return SlotDisplay(
        slot_id=slot.slot_id,
        source_path=slot.work_item.source_path,
        current_function=current_fn,
        total_functions=slot.work_item.total,
        matched_functions=slot.work_item.total - slot.current_remaining,
        remaining=slot.current_remaining,
        elapsed=elapsed,
        status=status,
        log_path=slot.log_path,
        log_size=slot.last_log_size,
        log_growing=slot.log_growing,
        process_alive=process_alive,
    )


def _terminate_process(proc: subprocess.Popen, timeout: float = 10.0) -> None:
    """Gracefully terminate a subprocess."""
    try:
        proc.terminate()
        proc.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=5.0)
    except OSError:
        pass


def _count_overall_progress(version: str = DEFAULT_VERSION) -> tuple[int, int]:
    """Return (total_functions, matched_functions) across all units."""
    index = load_repo_index(version)
    total = 0
    matched = 0
    for unit in index.units.values():
        if not unit.functions:
            continue
        counts = unit_match_counts(unit)
        total += counts["total"]
        matched += counts["matched"]
    return total, matched


def print_dry_run(work_queue: list[WorkItem], max_concurrent: int) -> None:
    """Print the work queue without launching any agents."""
    print(f"\n{'=' * 70}")
    print(f"  DRY RUN - Work Queue ({len(work_queue)} units)")
    print(f"  Max concurrent agents: {max_concurrent}")
    print(f"{'=' * 70}\n")

    print(f"  {'#':<4} {'Source File':<40} {'Remaining':<12} {'Total':<8} {'First Function'}")
    print(f"  {'-'*4} {'-'*40} {'-'*12} {'-'*8} {'-'*30}")

    for i, item in enumerate(work_queue, 1):
        print(
            f"  {i:<4} {item.source_path:<40} {item.remaining:<12} "
            f"{item.total:<8} {item.first_function}"
        )

    total_remaining = sum(w.remaining for w in work_queue)
    print(f"\n  Total unmatched functions: {total_remaining}")
    print(f"  Estimated batches (at {max_concurrent} parallel): "
          f"{(len(work_queue) + max_concurrent - 1) // max_concurrent}")
    print()


def write_summary(
    summary_path: str,
    completed: list[CompletedUnit],
    blocked: list[BlockedUnit],
    session_start: datetime,
    start_matched: int,
    end_matched: int,
    total: int,
) -> None:
    """Write session summary to JSON."""
    data = {
        "session_start": session_start.isoformat(),
        "session_end": datetime.now().isoformat(),
        "duration_seconds": (datetime.now() - session_start).total_seconds(),
        "functions_matched_start": start_matched,
        "functions_matched_end": end_matched,
        "functions_gained": end_matched - start_matched,
        "total_functions": total,
        "completed_units": [
            {
                "source_path": c.source_path,
                "total": c.total,
                "matched_during_session": c.matched_during,
                "duration_seconds": c.duration.total_seconds(),
            }
            for c in completed
        ],
        "blocked_units": [
            {
                "source_path": b.source_path,
                "total": b.total,
                "matched": b.matched,
                "blocked_functions": b.blocked_functions,
            }
            for b in blocked
        ],
    }
    os.makedirs(os.path.dirname(summary_path) or ".", exist_ok=True)
    with open(summary_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)
    print(f"\nSession summary written to: {summary_path}")


def build_summary_output_path(base_dir: str, session_start: datetime) -> str:
    timestamp = session_start.strftime("%Y%m%d_%H%M%S")
    summary_dir = os.path.join(base_dir, SUMMARY_DIRNAME)
    filename = f"pi_orchestrator_summary_{timestamp}.json"
    return os.path.join(summary_dir, filename)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Pi-agent orchestrator for parallel autonomous decompilation matching."
    )
    parser.add_argument("--max-concurrent", type=int, default=4,
                        help="Number of parallel agents (default: 4)")
    parser.add_argument("--include", action="append", default=[],
                        help="Only include units matching glob pattern (repeatable)")
    parser.add_argument("--exclude", action="append", default=[],
                        help="Exclude units matching glob pattern (repeatable)")
    parser.add_argument("--poll-interval", type=int, default=30,
                        help="Seconds between progress polls (default: 30)")
    parser.add_argument("--stall-timeout", type=int, default=30,
                        help="Minutes before declaring stall (default: 30)")
    parser.add_argument("--dry-run", action="store_true",
                        help="Show work queue without launching agents")
    parser.add_argument("--no-dashboard", action="store_true",
                        help="Disable live dashboard, use simple logging")
    parser.add_argument("--dasboard", "--dashboard", dest="dasboard",
                        help="Render a summary JSON, or the newest JSON in a summary directory, as a local HTML dashboard and open it")
    parser.add_argument("--provider", default="github-copilot",
                        help="Pi provider (default: github-copilot)")
    parser.add_argument("--model", default="claude-opus-4.6",
                        help="Model to use (default: claude-opus-4.6)")
    parser.add_argument("--thinking", default="high",
                        help="Thinking level: off, minimal, low, medium, high, xhigh (default: high)")
    parser.add_argument("--log-dir", default="tmp",
                        help="Directory for agent logs (default: tmp)")
    parser.add_argument("--version", default=DEFAULT_VERSION,
                        help=f"Project version (default: {DEFAULT_VERSION})")
    invocation_cwd = Path.cwd()
    args = parser.parse_args()

    if args.dasboard:
        summary_path = Path(args.dasboard).expanduser()
        if not summary_path.is_absolute():
            summary_path = invocation_cwd / summary_path
        try:
            dashboard_path = open_summary_dashboard(summary_path)
        except (FileNotFoundError, json.JSONDecodeError, OSError) as exc:
            print(f"Failed to build summary dashboard: {exc}", file=sys.stderr)
            return 1
        print(f"Summary dashboard written to: {dashboard_path}")
        return 0

    os.chdir(str(ROOT))

    # Build work queue
    print("Building work queue from report.json...")
    work_queue = build_work_queue(
        version=args.version,
        include_patterns=args.include or None,
        exclude_patterns=args.exclude or None,
    )

    if not work_queue:
        print("No units with unmatched functions found. Nothing to do!")
        return 0

    if args.dry_run:
        print_dry_run(work_queue, args.max_concurrent)
        return 0

    # Session state
    session_start = datetime.now()
    overall_total, session_start_matched = _count_overall_progress(args.version)
    active_slots: list[ActiveSlot] = []
    completed_units: list[CompletedUnit] = []
    blocked_units: list[BlockedUnit] = []
    queue_idx = 0
    shutdown_requested = False

    # Dashboard state
    dash_state = DashboardState(
        overall_total=overall_total,
        overall_matched=session_start_matched,
        session_start_matched=session_start_matched,
        queue_remaining=len(work_queue),
        max_slots=args.max_concurrent,
        poll_interval=args.poll_interval,
        session_start=session_start,
    )

    # Dashboard setup
    dashboard: Optional[Dashboard] = None
    use_dashboard = not args.no_dashboard and HAS_RICH
    if use_dashboard:
        dashboard = Dashboard(dash_state)

    # Signal handling for graceful shutdown
    def handle_signal(signum, frame):
        nonlocal shutdown_requested
        shutdown_requested = True

    signal.signal(signal.SIGINT, handle_signal)
    signal.signal(signal.SIGTERM, handle_signal)

    print(f"Starting orchestrator: {len(work_queue)} units, {args.max_concurrent} slots")

    if dashboard:
        dashboard.start()

    FAST_REFRESH_INTERVAL = 2  # seconds between visual dashboard updates
    last_full_poll = datetime.min  # force immediate first poll

    try:
        while not shutdown_requested:
            # Fill empty slots
            while len(active_slots) < args.max_concurrent and queue_idx < len(work_queue):
                item = work_queue[queue_idx]
                queue_idx += 1

                # Find an available slot ID
                used_ids = {s.slot_id for s in active_slots}
                slot_id = 1
                while slot_id in used_ids:
                    slot_id += 1

                slot = launch_subagent(
                    item, slot_id,
                    provider=args.provider,
                    model=args.model,
                    thinking=args.thinking,
                    log_dir=args.log_dir,
                )
                active_slots.append(slot)

                if not use_dashboard:
                    print(f"  [{slot_id}] Launched: {item.source_path} "
                          f"({item.remaining} remaining, first: {item.first_function})")

            if not active_slots:
                # No active work and no more queue items
                break

            # Wait for fast refresh interval
            for _ in range(FAST_REFRESH_INTERVAL * 10):
                if shutdown_requested:
                    break
                time.sleep(0.1)

            if shutdown_requested:
                break

            now = datetime.now()

            # --- Fast refresh: check log activity and process status ---
            for slot in active_slots:
                _check_log_activity(slot)

            # --- Slow poll: re-read report.json every poll_interval ---
            secs_since_poll = (now - last_full_poll).total_seconds()
            did_full_poll = False
            if secs_since_poll >= args.poll_interval:
                poll_progress(active_slots, version=args.version)
                overall_total, overall_matched = _count_overall_progress(args.version)
                dash_state.overall_total = overall_total
                dash_state.overall_matched = overall_matched
                dash_state.last_poll = now
                last_full_poll = now
                did_full_poll = True

            # Check for completed/stalled/crashed slots
            slots_to_remove: list[int] = []
            for i, slot in enumerate(active_slots):
                process_done = slot.process.poll() is not None
                is_stalled = (
                    (now - slot.last_progress_time).total_seconds()
                    > args.stall_timeout * 60
                )

                if process_done or slot.current_remaining == 0:
                    # Completed or exited
                    duration = now - slot.start_time
                    matched_during = slot.initial_remaining - slot.current_remaining
                    completed_units.append(CompletedUnit(
                        source_path=slot.work_item.source_path,
                        total=slot.work_item.total,
                        initial_remaining=slot.initial_remaining,
                        matched_during=matched_during,
                        duration=duration,
                    ))
                    if process_done and slot.current_remaining > 0:
                        # Process exited but functions remain - likely blocked/error
                        index = _reload_repo_index(args.version)
                        blocked_fns = []
                        for u in index.units.values():
                            if u.source_path == slot.work_item.source_path:
                                for fn in ranked_unit_functions(u, include_matched=False):
                                    blocked_fns.append(fn.name)
                                break
                        if blocked_fns:
                            blocked_units.append(BlockedUnit(
                                source_path=slot.work_item.source_path,
                                total=slot.work_item.total,
                                matched=slot.work_item.total - slot.current_remaining,
                                blocked_functions=blocked_fns,
                            ))
                    slots_to_remove.append(i)
                    if not use_dashboard:
                        status = "completed" if slot.current_remaining == 0 else "exited"
                        print(f"  [{slot.slot_id}] {status}: {slot.work_item.source_path} "
                              f"(+{matched_during} matched in {_format_duration_simple(duration)})")

                elif is_stalled:
                    # Stall timeout - kill and reassign
                    _terminate_process(slot.process)
                    duration = now - slot.start_time
                    matched_during = slot.initial_remaining - slot.current_remaining

                    index_data = _reload_repo_index(args.version)
                    blocked_fns = []
                    for u in index_data.units.values():
                        if u.source_path == slot.work_item.source_path:
                            for fn in ranked_unit_functions(u, include_matched=False):
                                blocked_fns.append(fn.name)
                            break
                    blocked_units.append(BlockedUnit(
                        source_path=slot.work_item.source_path,
                        total=slot.work_item.total,
                        matched=slot.work_item.total - slot.current_remaining,
                        blocked_functions=blocked_fns,
                    ))
                    slots_to_remove.append(i)
                    if not use_dashboard:
                        print(f"  [{slot.slot_id}] STALLED: {slot.work_item.source_path} "
                              f"(+{matched_during} matched, {slot.current_remaining} stuck)")

            # Remove finished slots (reverse order to preserve indices)
            for i in sorted(slots_to_remove, reverse=True):
                active_slots.pop(i)

            # Update dashboard state (every fast refresh)
            dash_state.queue_remaining = len(work_queue) - queue_idx
            dash_state.active_slots = [
                _slot_to_display(s, args.stall_timeout) for s in active_slots
            ]
            dash_state.completed = completed_units
            dash_state.blocked = blocked_units

            if dashboard:
                dashboard.update()
            elif not use_dashboard and did_full_poll:
                print_simple_status(dash_state)

    finally:
        # Graceful shutdown
        if dashboard:
            dashboard.stop()

        if active_slots:
            print("\nShutting down active agents...")
            for slot in active_slots:
                _terminate_process(slot.process)
                print(f"  [{slot.slot_id}] Terminated: {slot.work_item.source_path}")

        # Final progress
        overall_total, end_matched = _count_overall_progress(args.version)
        gained = end_matched - session_start_matched
        print(f"\nSession complete: +{gained} functions matched "
              f"({session_start_matched} -> {end_matched}/{overall_total})")
        print(f"  Completed units: {len(completed_units)}")
        print(f"  Blocked units: {len(blocked_units)}")

        # Write summary
        summary_path = build_summary_output_path(args.log_dir, session_start)
        write_summary(
            summary_path, completed_units, blocked_units,
            session_start, session_start_matched, end_matched, overall_total,
        )

    return 0


def _format_duration_simple(td: timedelta) -> str:
    total_secs = int(td.total_seconds())
    minutes, seconds = divmod(total_secs, 60)
    if minutes > 0:
        return f"{minutes}m {seconds:02d}s"
    return f"{seconds}s"


if __name__ == "__main__":
    raise SystemExit(main())
