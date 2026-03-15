#!/usr/bin/env python3
"""Rich-based terminal dashboard for the pi-agent orchestrator."""
from __future__ import annotations

import json
import os
import re
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from typing import Optional

try:
    from rich.console import Console, Group
    from rich.layout import Layout
    from rich.live import Live
    from rich.panel import Panel
    from rich.table import Table
    from rich.text import Text

    HAS_RICH = True
except ImportError:
    HAS_RICH = False


# How many lines of LLM output to show per slot
LOG_LINES_PER_SLOT = 6


@dataclass
class SlotDisplay:
    slot_id: int
    source_path: str
    current_function: str
    total_functions: int
    matched_functions: int
    remaining: int
    elapsed: timedelta
    status: str = "active"  # active, stalled, finishing, error, idle
    log_path: str = ""
    log_size: int = 0
    log_growing: bool = False
    process_alive: bool = True


@dataclass
class CompletedUnit:
    source_path: str
    total: int
    initial_remaining: int
    matched_during: int
    duration: timedelta


@dataclass
class BlockedUnit:
    source_path: str
    total: int
    matched: int
    blocked_functions: list[str] = field(default_factory=list)


@dataclass
class DashboardState:
    overall_total: int = 0
    overall_matched: int = 0
    session_start_matched: int = 0
    queue_remaining: int = 0
    max_slots: int = 4
    active_slots: list[SlotDisplay] = field(default_factory=list)
    completed: list[CompletedUnit] = field(default_factory=list)
    blocked: list[BlockedUnit] = field(default_factory=list)
    last_poll: Optional[datetime] = None
    poll_interval: int = 30
    session_start: Optional[datetime] = None


def _format_duration(td: timedelta) -> str:
    total_secs = int(td.total_seconds())
    if total_secs < 0:
        return "0s"
    hours, remainder = divmod(total_secs, 3600)
    minutes, seconds = divmod(remainder, 60)
    if hours > 0:
        return f"{hours}h {minutes:02d}m {seconds:02d}s"
    if minutes > 0:
        return f"{minutes}m {seconds:02d}s"
    return f"{seconds}s"


def _strip_ansi(text: str) -> str:
    """Remove ANSI escape codes from text."""
    return re.sub(r"\x1b\[[0-9;]*[a-zA-Z]|\x1b\][^\x07]*\x07|\x1b\[[?\d;]*[a-zA-Z]", "", text)


def _read_log_tail(log_path: str, max_lines: int = LOG_LINES_PER_SLOT) -> list[str]:
    """Read the last N meaningful events from a pi JSON-mode log file.

    Extracts text deltas, tool calls, and tool results into human-readable lines.
    Falls back to plain text parsing if the log isn't JSON.
    """
    if not log_path:
        return []
    try:
        with open(log_path, "r", encoding="utf-8", errors="replace") as f:
            f.seek(0, 2)
            size = f.tell()
            # Read last 32KB to find enough events
            f.seek(max(0, size - 32768))
            tail = f.read()
    except (OSError, IOError):
        return []

    raw_lines = tail.splitlines()
    if not raw_lines:
        return []

    # Try JSON parsing (pi --mode json)
    events: list[str] = []
    is_json = False
    for raw in raw_lines:
        raw = raw.strip()
        if not raw:
            continue
        if raw.startswith("{"):
            is_json = True
            try:
                obj = json.loads(raw)
            except json.JSONDecodeError:
                continue
            ev_type = obj.get("type", "")

            if ev_type == "message_update":
                ame = obj.get("assistantMessageEvent", {})
                ame_type = ame.get("type", "")
                if ame_type == "text_delta":
                    delta = ame.get("delta", "")
                    if delta.strip():
                        events.append(f"[white]{delta.strip()}[/white]")
                elif ame_type == "tool_call_start":
                    tc = ame.get("toolCall", {})
                    tool_name = tc.get("name", "?")
                    args = tc.get("arguments", {})
                    if tool_name == "bash":
                        cmd = args.get("command", "")
                        if cmd:
                            events.append(f"[cyan]$ {cmd[:90]}[/cyan]")
                        else:
                            events.append(f"[cyan]$ (bash)[/cyan]")
                    elif tool_name == "edit":
                        fp = args.get("file_path", "")
                        events.append(f"[yellow]edit {fp}[/yellow]")
                    elif tool_name == "read":
                        fp = args.get("file_path", "")
                        events.append(f"[blue]read {fp}[/blue]")
                    elif tool_name == "write":
                        fp = args.get("file_path", "")
                        events.append(f"[yellow]write {fp}[/yellow]")
                    else:
                        events.append(f"[magenta]{tool_name}[/magenta]")

            elif ev_type == "turn_end":
                msg = obj.get("message", {})
                tr = obj.get("toolResults", [])
                if tr:
                    events.append(f"[dim]--- turn end ({len(tr)} tool results) ---[/dim]")

    if is_json and events:
        return events[-max_lines:]

    # Fallback: plain text parsing
    clean = _strip_ansi(tail)
    lines = [line.strip() for line in clean.splitlines() if line.strip()]
    return lines[-max_lines:]


def _activity_icon(slot: SlotDisplay) -> str:
    """Return a single-char activity indicator."""
    if not slot.process_alive:
        if slot.remaining == 0:
            return "[bold green]✓[/bold green]"
        return "[red]✗[/red]"
    if slot.status == "error":
        return "[red]✗[/red]"
    if slot.status == "stalled":
        return "[yellow]⏸[/yellow]"
    if slot.status == "finishing":
        return "[bold green]✓[/bold green]"
    if slot.log_growing:
        tick = int(datetime.now().timestamp() * 3) % 4
        frames = ["◐", "◓", "◑", "◒"]
        return f"[bold green]{frames[tick]}[/bold green]"
    return "[dim yellow]○[/dim yellow]"


def _progress_bar(matched: int, total: int, width: int = 20) -> str:
    """Build a text-based progress bar with percentage."""
    if total == 0:
        return "[dim]no data[/dim]"
    pct = matched / total * 100
    filled = int(width * matched / total)
    empty = width - filled
    if pct >= 100:
        color = "bold green"
    elif pct >= 80:
        color = "green"
    elif pct >= 50:
        color = "yellow"
    else:
        color = "red"
    bar = f"[{color}]{'█' * filled}[/{color}][dim]{'░' * empty}[/dim]"
    return f"{bar} [{color}]{pct:5.1f}%[/{color}] ({matched}/{total})"


def _slot_header(slot: SlotDisplay) -> str:
    """Build a two-line header for a slot panel: file+function on top, progress below."""
    icon = _activity_icon(slot)
    progress = _progress_bar(slot.matched_functions, slot.total_functions, width=15)
    time_str = _format_duration(slot.elapsed)
    fn = slot.current_function
    if fn:
        fn_part = f"  →  [bold cyan]{fn}()[/bold cyan]"
    else:
        fn_part = ""
    line1 = f"{icon} [bold]{slot.source_path}[/bold]{fn_part}"
    line2 = f"  {progress}  [dim]{time_str}[/dim]"
    return f"{line1}\n{line2}"


def build_header(state: DashboardState) -> Panel:
    session_gained = state.overall_matched - state.session_start_matched
    active_count = sum(1 for s in state.active_slots if s.process_alive)

    elapsed = ""
    if state.session_start:
        elapsed = _format_duration(datetime.now() - state.session_start)

    overall_bar = _progress_bar(state.overall_matched, state.overall_total, width=30)

    content = (
        f"[bold]Progress:[/bold] {overall_bar}  "
        f"[bold green]+{session_gained}[/bold green] this session\n"
        f"[bold]Agents:[/bold] {active_count}/{state.max_slots} active  "
        f"[bold]Queue:[/bold] {state.queue_remaining} units  "
        f"[bold]Completed:[/bold] {len(state.completed)}  "
        f"[bold]Blocked:[/bold] {len(state.blocked)}  "
        f"[dim]Uptime: {elapsed}[/dim]"
    )
    return Panel(
        content,
        title="[bold cyan]Crash WOC Decomp - Pi Agent Orchestrator[/bold cyan]",
        border_style="cyan",
    )


def build_slot_panel(slot: SlotDisplay) -> Panel:
    """Build a panel for one active agent slot showing header + live LLM output."""
    header = _slot_header(slot)
    log_lines = _read_log_tail(slot.log_path, max_lines=LOG_LINES_PER_SLOT)

    if log_lines:
        # Truncate long lines and escape rich markup in log content
        display_lines = []
        for line in log_lines:
            # Escape [ and ] so rich doesn't interpret them as markup
            safe = line.replace("[", "\\[").replace("]", "\\]")
            if len(safe) > 100:
                safe = safe[:97] + "..."
            display_lines.append(f"[dim]{safe}[/dim]")
        body = "\n".join(display_lines)
    elif slot.log_size == 0 and slot.process_alive:
        body = "[dim italic]Waiting for LLM output...[/dim italic]"
    elif not slot.process_alive and slot.log_size == 0:
        body = "[red]Agent exited without output[/red]"
    else:
        body = "[dim]...[/dim]"

    # Border color based on status
    if slot.status == "finishing" or (not slot.process_alive and slot.remaining == 0):
        border = "green"
    elif slot.status == "error" or (not slot.process_alive and slot.remaining > 0):
        border = "red"
    elif slot.status == "stalled":
        border = "yellow"
    elif slot.log_growing:
        border = "bright_green"
    else:
        border = "blue"

    return Panel(
        f"{header}\n{'─' * 60}\n{body}",
        border_style=border,
    )


def build_idle_slot_panel(slot_id: int) -> Panel:
    """Build a compact panel for an idle slot."""
    return Panel(
        f"[dim]Slot {slot_id} — idle[/dim]",
        border_style="dim",
        height=3,
    )


def build_footer(state: DashboardState) -> Panel:
    poll_str = state.last_poll.strftime("%H:%M:%S") if state.last_poll else "-"
    if state.last_poll:
        next_poll_in = state.poll_interval - (datetime.now() - state.last_poll).total_seconds()
        next_str = f"{max(0, int(next_poll_in))}s"
    else:
        next_str = "-"

    now_str = datetime.now().strftime("%H:%M:%S")

    # Compact completed/blocked summary
    parts = [f"[bold]{now_str}[/bold]"]
    parts.append(f"Poll: {poll_str} (next: {next_str})")

    if state.completed:
        last = state.completed[-1]
        parts.append(
            f"[green]Last completed: {last.source_path} "
            f"(+{last.matched_during}) in {_format_duration(last.duration)}[/green]"
        )
    if state.blocked:
        last = state.blocked[-1]
        names = ", ".join(last.blocked_functions[:2])
        parts.append(f"[yellow]Last blocked: {last.source_path} ({names})[/yellow]")

    return Panel(" | ".join(parts), border_style="dim")


def render_dashboard(state: DashboardState) -> Layout:
    """Build the full dashboard layout from current state."""
    layout = Layout()

    # Build slot panels
    slot_panels: list[Layout] = []
    used_slot_ids: set[int] = set()

    for slot in state.active_slots:
        used_slot_ids.add(slot.slot_id)
        # Active slots get a taller panel with log output
        panel = build_slot_panel(slot)
        slot_panels.append(Layout(panel, size=LOG_LINES_PER_SLOT + 6))

    # Fill remaining slots as idle (compact)
    for i in range(1, state.max_slots + 1):
        if i not in used_slot_ids:
            slot_panels.append(Layout(build_idle_slot_panel(i), size=3))

    layout.split_column(
        Layout(build_header(state), name="header", size=5),
        *slot_panels,
        Layout(build_footer(state), name="footer", size=3),
    )
    return layout


class Dashboard:
    """Manages a live-updating terminal dashboard."""

    def __init__(self, state: DashboardState) -> None:
        if not HAS_RICH:
            raise ImportError(
                "The 'rich' package is required for the dashboard. "
                "Install it with: pip install rich"
            )
        self.state = state
        self.console = Console()
        self.live: Optional[Live] = None

    def start(self) -> None:
        self.live = Live(
            render_dashboard(self.state),
            console=self.console,
            refresh_per_second=4,
            screen=True,
        )
        self.live.start()

    def update(self) -> None:
        if self.live:
            self.live.update(render_dashboard(self.state))

    def stop(self) -> None:
        if self.live:
            self.live.stop()
            self.live = None


def print_simple_status(state: DashboardState) -> None:
    """Fallback status line for --no-dashboard mode."""
    session_gained = state.overall_matched - state.session_start_matched
    active = len(state.active_slots)
    slots_info = []
    for s in state.active_slots:
        pct = (s.matched_functions / s.total_functions * 100) if s.total_functions else 0
        activity = "ACTIVE" if s.log_growing else "wait"
        if not s.process_alive:
            activity = "DEAD"
        slots_info.append(
            f"  [{s.slot_id}] {activity:>6} {s.source_path} "
            f"{s.matched_functions}/{s.total_functions} ({pct:.1f}%)"
        )
    poll_str = state.last_poll.strftime("%H:%M:%S") if state.last_poll else "-"
    print(
        f"[{poll_str}] matched={state.overall_matched}/{state.overall_total} "
        f"(+{session_gained}) active={active}/{state.max_slots} queue={state.queue_remaining}"
    )
    for line in slots_info:
        print(line)
