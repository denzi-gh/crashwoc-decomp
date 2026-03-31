#!/usr/bin/env python3
"""Crash WOC Decomp Dashboard – multi-agent orchestration TUI.

Launch with:  python tools/ai_dashboard.py
              python tools/ai_dashboard.py --category gameplay
              python tools/ai_dashboard.py --max-agents 2
              python tools/ai_dashboard.py --auto 4
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import signal
import subprocess
import sys
import threading
import time
import uuid
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path
from typing import Any, Optional

# Ensure tools/ is importable
sys.path.insert(0, str(Path(__file__).resolve().parent))

from ai_common import DEFAULT_VERSION, find_units, format_percent, load_repo_index
from ai_pick_unit import (
    LOCK_DIR,
    claim_unit,
    eligible_units,
    read_locks,
    release_agent,
    unit_remaining_work,
)
from ai_status import aggregate_categories, safe_float

try:
    from rich.console import Console
    from rich.layout import Layout
    from rich.live import Live
    from rich.panel import Panel
    from rich.table import Table
    from rich.text import Text
    from rich import box
    from rich.spinner import Spinner
    from rich.columns import Columns
except ImportError:
    print(
        "This script requires the 'rich' package.  Install with: pip install rich",
        file=sys.stderr,
    )
    sys.exit(1)

ROOT = Path(__file__).resolve().parent.parent
MAX_AGENTS = 4
REFRESH_INTERVAL = 1.0  # seconds between dashboard refreshes

SPINNER_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]


def resolve_pi_cmd() -> str:
    """Resolve the full path to the pi executable (handles .cmd on Windows)."""
    pi_path = shutil.which("pi")
    if pi_path:
        return pi_path
    if sys.platform == "win32":
        pi_cmd = shutil.which("pi.cmd")
        if pi_cmd:
            return pi_cmd
    return "pi"


PI_CMD = resolve_pi_cmd()


# ---------------------------------------------------------------------------
# Agent state
# ---------------------------------------------------------------------------


@dataclass
class FunctionSnapshot:
    name: str
    address: int
    size: int
    match_pct: float | None


@dataclass
class AgentSlot:
    agent_id: str
    unit_name: str
    source_path: str | None = None
    pid: int | None = None
    process: subprocess.Popen | None = None
    started_at: float = 0.0
    finished_at: float = 0.0  # frozen when agent stops/finishes
    status: str = "starting"  # starting | running | finished | failed | stopped
    session_file: str | None = None
    # Match tracking
    start_match_pct: float | None = None
    current_match_pct: float | None = None
    start_functions: list[FunctionSnapshot] = field(default_factory=list)
    # Report (generated when agent finishes)
    report_text: str = ""
    # Extracted activity info (updated each tick)
    current_action: str = ""
    tool_count: int = 0


def get_unit_match_pct(unit_name: str, version: str) -> float | None:
    """Look up the current matched_code_percent for a unit from report.json."""
    try:
        matches = find_units(unit_name, version=version)
        if matches:
            return safe_float(matches[0].measures.get("matched_code_percent"))
    except Exception:
        pass
    return None


def snapshot_functions(unit_name: str, version: str) -> list[FunctionSnapshot]:
    """Take a snapshot of all function match states for a unit."""
    try:
        matches = find_units(unit_name, version=version)
        if not matches:
            return []
        return [
            FunctionSnapshot(
                name=fn.name,
                address=fn.address,
                size=fn.size,
                match_pct=fn.fuzzy_match_percent,
            )
            for fn in matches[0].functions
        ]
    except Exception:
        return []


def generate_report(slot: AgentSlot, version: str) -> str:
    """Generate a per-function report comparing start vs end state."""
    end_functions = snapshot_functions(slot.unit_name, version)
    start_by_name = {f.name: f for f in slot.start_functions}
    end_by_name = {f.name: f for f in end_functions}

    unit_display = slot.source_path or slot.unit_name
    elapsed = format_elapsed(slot.started_at, slot.finished_at)
    start_pct = slot.start_match_pct
    end_pct = slot.current_match_pct

    lines: list[str] = []
    lines.append(f"Report: {unit_display}")
    lines.append(f"Agent:  {slot.agent_id}  |  Status: {slot.status}  |  Duration: {elapsed}")
    if start_pct is not None and end_pct is not None:
        lines.append(f"Unit:   {format_percent(start_pct)} -> {format_percent(end_pct)}")
    lines.append("")

    # Categorize functions
    improved: list[str] = []
    matched: list[str] = []
    blocked: list[str] = []
    unchanged: list[str] = []

    all_names = list(dict.fromkeys(
        [f.name for f in slot.start_functions] + [f.name for f in end_functions]
    ))

    for name in all_names:
        before = start_by_name.get(name)
        after = end_by_name.get(name)
        b_pct = before.match_pct if before else None
        a_pct = after.match_pct if after else None
        size = (after.size if after else before.size) if (after or before) else 0

        b_str = format_percent(b_pct)
        a_str = format_percent(a_pct)

        if a_pct is not None and a_pct >= 100.0:
            if b_pct is not None and b_pct >= 100.0:
                matched.append(f"  {name:<36} {a_str:>7}  (already matched)")
            else:
                improved.append(f"  {name:<36} {b_str:>7} -> {a_str:>7}  [NEW MATCH]")
        elif b_pct is not None and a_pct is not None and a_pct > b_pct + 0.1:
            improved.append(f"  {name:<36} {b_str:>7} -> {a_str:>7}  (improved)")
        elif a_pct is not None and a_pct < 100.0:
            if b_pct is not None and abs((a_pct or 0) - (b_pct or 0)) < 0.1:
                blocked.append(f"  {name:<36} {a_str:>7}  (blocked)")
            else:
                unchanged.append(f"  {name:<36} {b_str:>7} -> {a_str:>7}")
        else:
            unchanged.append(f"  {name:<36} {a_str:>7}")

    if improved:
        lines.append(f"IMPROVED ({len(improved)}):")
        lines.extend(improved)
        lines.append("")
    if blocked:
        lines.append(f"BLOCKED ({len(blocked)}):")
        lines.extend(blocked)
        lines.append("")
    if matched:
        lines.append(f"ALREADY MATCHED ({len(matched)}):")
        lines.extend(matched)
        lines.append("")
    if unchanged:
        lines.append(f"UNCHANGED ({len(unchanged)}):")
        lines.extend(unchanged)
        lines.append("")

    return "\n".join(lines)


def save_report(slot: AgentSlot, report_text: str) -> Path:
    """Save the report to .pi/dashboard-reports/."""
    report_dir = ROOT / ".pi" / "dashboard-reports"
    report_dir.mkdir(parents=True, exist_ok=True)
    report_path = report_dir / f"{slot.agent_id}.txt"
    report_path.write_text(report_text, encoding="utf-8")
    return report_path


# ---------------------------------------------------------------------------
# Prompt builder for pi
# ---------------------------------------------------------------------------


def build_agent_prompt(unit_name: str, source_path: str | None) -> str:
    target = source_path or unit_name
    return f"""\
/skill:decomp-match

My assigned unit is `{target}`.

Start the decomp matching workflow now:
1. Run `python tools/ai_match_plan.py {target}` to see the function backlog
2. Pick the first unmatched function
3. Follow the decomp-match skill workflow: edit one function, build, check, repeat
4. Keep going autonomously until all functions are matched or blocked
5. When done with all functions, print a final summary

Work autonomously. Do not ask questions. Keep going until every function is matched or blocked.
"""


# ---------------------------------------------------------------------------
# Session parsing – extract current activity from .jsonl session
# ---------------------------------------------------------------------------

_ACTION_MAP = {
    "read": "reading",
    "edit": "editing",
    "write": "writing",
    "bash": "running cmd",
    "search": "searching",
    "grep": "searching",
    "glob": "finding files",
}


def parse_session_activity(session_path: str | None) -> tuple[str, int]:
    """Parse the session .jsonl to extract (current_action, tool_count)."""
    if not session_path:
        return "", 0
    sf = Path(session_path)
    if not sf.is_file():
        return "", 0

    try:
        data = sf.read_bytes()
    except OSError:
        return "", 0

    try:
        text = data.decode("utf-8", errors="replace")
    except Exception:
        return "", 0

    lines = text.strip().split("\n")

    current_action = ""
    tool_count = 0

    # Scan all lines for tool count
    for line in lines:
        try:
            msg = json.loads(line)
        except (json.JSONDecodeError, ValueError):
            continue
        if msg.get("type") != "message":
            continue
        content = msg.get("message", {}).get("content", [])
        if not isinstance(content, list):
            continue
        for block in content:
            if block.get("type") == "toolCall":
                tool_count += 1

    # Scan last ~20 lines for current action
    recent_lines = lines[-20:] if len(lines) > 20 else lines
    for line in reversed(recent_lines):
        try:
            msg = json.loads(line)
        except (json.JSONDecodeError, ValueError):
            continue
        if msg.get("type") != "message":
            continue

        content = msg.get("message", {}).get("content", [])
        if not isinstance(content, list):
            continue

        for block in content:
            if block.get("type") == "toolCall" and not current_action:
                tool_name = block.get("name", "")
                current_action = _ACTION_MAP.get(tool_name, tool_name)
                break

        if current_action:
            break

    return current_action, tool_count


# ---------------------------------------------------------------------------
# Agent lifecycle
# ---------------------------------------------------------------------------


def _log_reader(proc: subprocess.Popen, log_path: Path) -> None:
    """Background thread: reads from proc.stdout and appends to log file."""
    try:
        with open(log_path, "ab") as log_file:
            while True:
                chunk = proc.stdout.read(512)
                if not chunk:
                    break
                log_file.write(chunk)
                log_file.flush()
    except (OSError, ValueError):
        pass


def start_agent(
    unit_name: str,
    source_path: str | None,
    category: str | None,
    version: str,
    model: str | None,
    provider: str | None,
) -> AgentSlot:
    agent_id = f"agent-{uuid.uuid4().hex[:8]}"
    claim_unit(unit_name, agent_id)

    prompt = build_agent_prompt(unit_name, source_path)

    session_dir = ROOT / ".pi" / "dashboard-sessions"
    session_dir.mkdir(parents=True, exist_ok=True)
    session_file = session_dir / f"{agent_id}.jsonl"

    prompt_dir = ROOT / ".pi" / "dashboard-prompts"
    prompt_dir.mkdir(parents=True, exist_ok=True)
    prompt_path = prompt_dir / f"{agent_id}.md"
    prompt_path.write_text(prompt, encoding="utf-8")

    cmd = [PI_CMD]
    if provider:
        cmd.extend(["--provider", provider])
    else:
        cmd.extend(["--provider", "copilot"])
    if model:
        cmd.extend(["--model", model])
    cmd.extend([
        "--session", str(session_file),
        "--print",
        "--thinking", "medium",
        f"@{prompt_path}",
    ])

    log_dir = ROOT / ".pi" / "dashboard-logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    log_path = log_dir / f"{agent_id}.log"

    env = {**os.environ, "PYTHONUNBUFFERED": "1"}
    proc = subprocess.Popen(
        cmd,
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        env=env,
        creationflags=subprocess.CREATE_NEW_PROCESS_GROUP if sys.platform == "win32" else 0,
    )

    reader = threading.Thread(target=_log_reader, args=(proc, log_path), daemon=True)
    reader.start()

    start_pct = get_unit_match_pct(unit_name, version)
    start_fns = snapshot_functions(unit_name, version)

    return AgentSlot(
        agent_id=agent_id,
        unit_name=unit_name,
        source_path=source_path,
        pid=proc.pid,
        process=proc,
        started_at=time.time(),
        status="running",
        session_file=str(session_file),
        start_match_pct=start_pct,
        current_match_pct=start_pct,
        start_functions=start_fns,
    )


def stop_agent(slot: AgentSlot) -> None:
    if slot.process and slot.process.poll() is None:
        try:
            if sys.platform == "win32":
                slot.process.terminate()
            else:
                os.killpg(os.getpgid(slot.process.pid), signal.SIGTERM)
        except (OSError, ProcessLookupError):
            pass
    release_agent(slot.agent_id)
    slot.status = "stopped"
    slot.finished_at = time.time()


def check_agent(slot: AgentSlot, version: str) -> None:
    if slot.process is None:
        return
    rc = slot.process.poll()
    if rc is not None:
        slot.status = "finished" if rc == 0 else f"failed(rc={rc})"
        slot.finished_at = time.time()
        # Reload report to get fresh match % after agent's builds
        load_repo_index.cache_clear()
        pct = get_unit_match_pct(slot.unit_name, version)
        if pct is not None:
            slot.current_match_pct = pct
        # Generate per-function report
        report = generate_report(slot, version)
        slot.report_text = report
        save_report(slot, report)
        release_agent(slot.agent_id)


def update_agent_activity(slot: AgentSlot, version: str) -> None:
    """Refresh current_action and match % from session file and report."""
    if slot.status != "running":
        return
    action, tools = parse_session_activity(slot.session_file)
    if action:
        slot.current_action = action
    slot.tool_count = tools
    # Refresh match % (report.json may have changed after a build)
    pct = get_unit_match_pct(slot.unit_name, version)
    if pct is not None:
        slot.current_match_pct = pct


def format_elapsed(start: float, end: float = 0.0) -> str:
    now = end if end else time.time()
    elapsed = int(now - start)
    if elapsed < 60:
        return f"{elapsed}s"
    m, s = divmod(elapsed, 60)
    if m < 60:
        return f"{m}m{s:02d}s"
    h, m = divmod(m, 60)
    return f"{h}h{m:02d}m"


# ---------------------------------------------------------------------------
# Non-blocking keyboard input
# ---------------------------------------------------------------------------

if sys.platform == "win32":
    import msvcrt

    def get_key() -> str | None:
        """Return a single key press without blocking, or None."""
        if msvcrt.kbhit():
            ch = msvcrt.getwch()
            return ch
        return None
else:
    import select
    import termios
    import tty

    _old_settings = None

    def _enable_raw() -> None:
        global _old_settings
        try:
            _old_settings = termios.tcgetattr(sys.stdin)
            tty.setcbreak(sys.stdin.fileno())
        except termios.error:
            _old_settings = None

    def _restore_term() -> None:
        global _old_settings
        if _old_settings is not None:
            try:
                termios.tcsetattr(sys.stdin, termios.TCSADRAIN, _old_settings)
            except termios.error:
                pass
            _old_settings = None

    def get_key() -> str | None:
        """Return a single key press without blocking, or None."""
        if select.select([sys.stdin], [], [], 0)[0]:
            return sys.stdin.read(1)
        return None


# ---------------------------------------------------------------------------
# TUI rendering
# ---------------------------------------------------------------------------


def make_header(tick: int, agent_count: int, running_count: int, max_agents: int) -> Panel:
    now = datetime.now().strftime("%H:%M:%S")
    spinner = SPINNER_FRAMES[tick % len(SPINNER_FRAMES)]
    if running_count > 0:
        status = f"[bold green]{spinner} {running_count} running[/]"
    else:
        status = "[dim]idle[/]"

    header = Text.from_markup(
        f"  [bold cyan]CRASH WOC DECOMP[/]  [dim]|[/]  {now}  [dim]|[/]  "
        f"Agents: {status} [dim]/ {max_agents} max[/]  [dim]|[/]  "
        f"[dim]Press [bold white]?[/bold white] for help[/]"
    )
    return Panel(header, style="cyan", box=box.HEAVY, padding=(0, 1))


def make_agents_panel(agents: list[AgentSlot], tick: int) -> Panel:
    if not agents:
        content = Text.from_markup(
            "\n  [dim]No agents running. Press [bold white]a[/bold white] to add one.[/]\n"
        )
        return Panel(content, title="[bold]Agents[/]", border_style="blue", box=box.ROUNDED)

    table = Table(
        box=None,
        show_header=True,
        header_style="bold dim",
        expand=True,
        padding=(0, 1),
    )
    table.add_column("#", width=2, justify="right")
    table.add_column("Status", width=10)
    table.add_column("Unit", min_width=18, ratio=2)
    table.add_column("Match", width=16, justify="right")
    table.add_column("Activity", min_width=12, ratio=1)
    table.add_column("Calls", width=5, justify="right")
    table.add_column("Time", width=7, justify="right")

    for i, slot in enumerate(agents, 1):
        spinner = SPINNER_FRAMES[tick % len(SPINNER_FRAMES)]

        if slot.status == "running":
            status_str = f"[bold green]{spinner} running[/]"
        elif slot.status == "finished":
            status_str = "[bold blue]✓ done[/]"
        elif slot.status == "stopped":
            status_str = "[dim]■ stopped[/]"
        elif slot.status == "starting":
            status_str = f"[yellow]{spinner} start…[/]"
        else:
            status_str = f"[red]✗ {slot.status[:8]}[/]"

        unit_display = slot.source_path or slot.unit_name
        # Shorten path: src/nusound/sfx.c → nusound/sfx.c
        if unit_display.startswith("src/"):
            unit_display = unit_display[4:]

        # Match column: "56.2%" while running, "56.2% → 67.8%" when done
        start_pct = slot.start_match_pct
        cur_pct = slot.current_match_pct
        if slot.status in ("finished", "stopped") and start_pct is not None and cur_pct is not None:
            delta = cur_pct - start_pct
            if delta > 0.1:
                match_str = f"[green]{format_percent(start_pct)} → {format_percent(cur_pct)}[/]"
            elif delta < -0.1:
                match_str = f"[red]{format_percent(start_pct)} → {format_percent(cur_pct)}[/]"
            else:
                match_str = f"[dim]{format_percent(cur_pct)} (no change)[/]"
        elif cur_pct is not None:
            match_str = format_percent(cur_pct)
        else:
            match_str = "[dim]-[/]"

        action = slot.current_action if slot.status == "running" else ""
        tools_str = str(slot.tool_count) if slot.tool_count else "-"
        elapsed = format_elapsed(slot.started_at, slot.finished_at) if slot.started_at else "-"

        table.add_row(
            str(i),
            status_str,
            f"[bold]{unit_display}[/]",
            match_str,
            action,
            tools_str,
            elapsed,
        )

    return Panel(table, title="[bold]Agents[/]", border_style="blue", box=box.ROUNDED)


def make_progress_panel(version: str) -> Panel:
    try:
        index = load_repo_index(version)
        categories = aggregate_categories(index)
    except Exception:
        return Panel("[red]Could not load report.json[/]", title="Progress", box=box.ROUNDED)

    table = Table(box=None, show_header=True, header_style="bold dim", expand=True, padding=(0, 1))
    table.add_column("Category", ratio=2)
    table.add_column("Fn", justify="right", width=10)
    table.add_column("Code%", justify="right", width=7)

    total_matched = 0
    total_functions = 0

    for cat in categories:
        matched_fn = cat.get("matched_functions", 0)
        total_fn = cat.get("total_functions", 0)
        total_matched += matched_fn
        total_functions += total_fn
        code_pct = safe_float(cat.get("matched_code_percent"))

        if code_pct is not None:
            if code_pct >= 80:
                color = "green"
            elif code_pct >= 40:
                color = "yellow"
            else:
                color = "red"
            pct_str = f"[{color}]{format_percent(code_pct)}[/{color}]"
        else:
            pct_str = "[dim]-[/]"

        name = cat.get("name") or cat.get("id", "?")
        table.add_row(
            name,
            f"{matched_fn}/{total_fn}",
            pct_str,
        )

    # Summary row
    overall_pct = (total_matched / total_functions * 100) if total_functions else 0
    table.add_row(
        "[bold]Total[/]",
        f"[bold]{total_matched}/{total_functions}[/]",
        f"[bold]{format_percent(overall_pct)}[/]",
        style="on grey11" if sys.platform != "win32" else "",
    )

    return Panel(table, title="[bold]Progress[/]", border_style="green", box=box.ROUNDED)


def make_help_panel() -> Panel:
    help_text = Text.from_markup(
        " [bold cyan]a[/] add agent  "
        "[bold cyan]s[/]/[bold cyan]1-4[/] stop  "
        "[bold cyan]r[/] reload report  "
        "[bold cyan]q[/] quit"
    )
    return Panel(help_text, box=box.ROUNDED, style="dim", padding=(0, 1))


def make_report_panel(agents: list[AgentSlot]) -> Panel | None:
    """Build a panel showing the most recent finished agent's report."""
    # Find the most recently finished agent with a report
    finished = [a for a in agents if a.report_text and a.finished_at > 0]
    if not finished:
        return None
    latest = max(finished, key=lambda a: a.finished_at)

    # Colorize the report text
    lines: list[str] = []
    for line in latest.report_text.split("\n"):
        if "[NEW MATCH]" in line:
            lines.append(f"[bold green]{line}[/]")
        elif "(improved)" in line:
            lines.append(f"[green]{line}[/]")
        elif "(blocked)" in line:
            lines.append(f"[yellow]{line}[/]")
        elif "(already matched)" in line:
            lines.append(f"[dim]{line}[/]")
        elif line.startswith("IMPROVED"):
            lines.append(f"[bold green]{line}[/]")
        elif line.startswith("BLOCKED"):
            lines.append(f"[bold yellow]{line}[/]")
        elif line.startswith("ALREADY"):
            lines.append(f"[dim]{line}[/]")
        elif line.startswith("UNCHANGED"):
            lines.append(f"[dim]{line}[/]")
        elif line.startswith("Report:"):
            lines.append(f"[bold]{line}[/]")
        else:
            lines.append(line)

    content = Text.from_markup("\n".join(lines))
    return Panel(
        content,
        title=f"[bold]Last Report: {latest.source_path or latest.unit_name}[/]",
        border_style="magenta",
        box=box.ROUNDED,
    )


def make_layout(
    agents: list[AgentSlot],
    version: str,
    max_agents: int,
    tick: int,
    message: str,
) -> Layout:
    running = sum(1 for a in agents if a.status == "running")
    report_panel = make_report_panel(agents)

    layout = Layout()

    if report_panel:
        layout.split_column(
            Layout(name="header", size=3),
            Layout(name="top", ratio=1),
            Layout(name="report", ratio=1),
            Layout(name="footer", size=3),
        )
        layout["report"].update(report_panel)
    else:
        layout.split_column(
            Layout(name="header", size=3),
            Layout(name="top"),
            Layout(name="footer", size=3),
        )

    layout["header"].update(make_header(tick, len(agents), running, max_agents))

    # Top: agents on left, progress on right
    top = Layout()
    top.split_row(
        Layout(name="agents", ratio=3),
        Layout(name="progress", ratio=2),
    )
    top["agents"].update(make_agents_panel(agents, tick))
    top["progress"].update(make_progress_panel(version))
    layout["top"].update(top)

    if message:
        footer_text = Text.from_markup(f"  [bold yellow]{message}[/]")
        layout["footer"].update(Panel(footer_text, box=box.ROUNDED, style="yellow", padding=(0, 1)))
    else:
        layout["footer"].update(make_help_panel())

    return layout


# ---------------------------------------------------------------------------
# Unit selection (non-interactive for auto, simple prompt for manual)
# ---------------------------------------------------------------------------


def pick_unit_for_agent(version: str, category: str | None) -> tuple[str, str | None] | None:
    """Pick a random eligible unit."""
    from ai_pick_unit import pick_random_weighted
    index = load_repo_index(version)
    locked = set(read_locks().keys())
    candidates = list(eligible_units(index, version, category, locked))
    if not candidates:
        return None
    unit = pick_random_weighted(candidates)
    if unit is None:
        return None
    return unit.normalized_name, unit.source_path


# ---------------------------------------------------------------------------
# Interactive loop
# ---------------------------------------------------------------------------


def run_dashboard(
    version: str,
    category: str | None,
    max_agents: int,
    model: str | None,
    provider: str | None,
    auto_start: int,
) -> None:
    console = Console()
    agents: list[AgentSlot] = []
    message = ""
    message_until = 0.0
    tick = 0
    # How many agents we *want* running.  Starts at auto_start, decreases
    # when the user manually stops an agent, increases with 'a'.
    desired_count = auto_start

    # Clean stale locks
    if LOCK_DIR.is_dir():
        for lock_file in LOCK_DIR.glob("*.json"):
            lock_file.unlink(missing_ok=True)

    def set_message(msg: str, duration: float = 3.0) -> None:
        nonlocal message, message_until
        message = msg
        message_until = time.time() + duration

    def add_agent_auto(user_initiated: bool = False) -> None:
        nonlocal desired_count
        running = [a for a in agents if a.status == "running"]
        if len(running) >= max_agents:
            set_message(f"Max {max_agents} agents already running")
            return
        result = pick_unit_for_agent(version, category)
        if result is None:
            set_message("No eligible units available")
            return
        unit_name, source_path = result
        try:
            slot = start_agent(unit_name, source_path, category, version, model, provider)
            agents.append(slot)
            if user_initiated:
                desired_count = max(desired_count, len([a for a in agents if a.status == "running"]))
            set_message(f"Started agent on {source_path or unit_name}")
        except Exception as e:
            set_message(f"Failed: {e}")

    def stop_agent_by_index(idx: int) -> None:
        nonlocal desired_count
        if idx < 0 or idx >= len(agents):
            set_message("Invalid agent number")
            return
        slot = agents[idx]
        if slot.status == "running":
            stop_agent(slot)
            # Reduce desired count so auto-replace won't respawn it
            desired_count = max(desired_count - 1, 0)
            set_message(f"Stopped agent #{idx + 1} ({slot.unit_name})")
        else:
            set_message(f"Agent #{idx + 1} already {slot.status}")

    # Auto-start agents
    for _ in range(min(auto_start, max_agents)):
        add_agent_auto()

    # Enable raw mode on Unix
    if sys.platform != "win32":
        _enable_raw()

    try:
        with Live(
            make_layout(agents, version, max_agents, tick, message),
            console=console,
            refresh_per_second=1,
            screen=True,
        ) as live:
            while True:
                # Check agent statuses and update activity
                for slot in agents:
                    if slot.status in ("running", "starting"):
                        check_agent(slot, version)
                    if slot.status == "running":
                        update_agent_activity(slot, version)

                # Auto-replace agents that finished naturally (not manually stopped)
                if desired_count > 0:
                    running = [a for a in agents if a.status == "running"]
                    if len(running) < min(desired_count, max_agents):
                        add_agent_auto()

                # Clear expired messages
                if message and time.time() > message_until:
                    message = ""

                # Update display
                tick += 1
                live.update(make_layout(agents, version, max_agents, tick, message))

                # Handle keyboard input (check multiple times per second)
                deadline = time.time() + REFRESH_INTERVAL
                while time.time() < deadline:
                    key = get_key()
                    if key is not None:
                        key = key.lower()
                        if key == "q":
                            return
                        elif key == "a":
                            add_agent_auto(user_initiated=True)
                            live.update(make_layout(agents, version, max_agents, tick, message))
                        elif key == "s":
                            # Stop the most recently started running agent
                            for idx in range(len(agents) - 1, -1, -1):
                                if agents[idx].status == "running":
                                    stop_agent_by_index(idx)
                                    break
                            else:
                                set_message("No running agents to stop")
                            live.update(make_layout(agents, version, max_agents, tick, message))
                        elif key in ("1", "2", "3", "4"):
                            stop_agent_by_index(int(key) - 1)
                            live.update(make_layout(agents, version, max_agents, tick, message))
                        elif key == "r":
                            load_repo_index.cache_clear()
                            set_message("Report reloaded")
                            live.update(make_layout(agents, version, max_agents, tick, message))
                        elif key == "?":
                            set_message(
                                "a=add  s=stop last  1-4=stop #N  r=reload  q=quit",
                                duration=5.0,
                            )
                            live.update(make_layout(agents, version, max_agents, tick, message))
                    time.sleep(0.05)

    except KeyboardInterrupt:
        pass
    finally:
        if sys.platform != "win32":
            _restore_term()
        # Cleanup: stop all agents
        console.print("\n[bold]Shutting down agents...[/]")
        for slot in agents:
            if slot.status == "running":
                stop_agent(slot)
        if LOCK_DIR.is_dir():
            for lock_file in LOCK_DIR.glob("*.json"):
                lock_file.unlink(missing_ok=True)
        console.print("[green]Dashboard closed.[/]")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Crash WOC Decomp Dashboard – orchestrate multiple pi agents.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""\
Examples:
  python tools/ai_dashboard.py                     # Interactive dashboard
  python tools/ai_dashboard.py --auto 4            # Auto-start 4 agents
  python tools/ai_dashboard.py --category gameplay  # Only gameplay units
  python tools/ai_dashboard.py --model claude-sonnet-4-20250514
""",
    )
    parser.add_argument("--version", default=DEFAULT_VERSION, help="Build version (default: GCBE7D)")
    parser.add_argument("--category", default=None, help="Filter units by progress category")
    parser.add_argument("--max-agents", type=int, default=MAX_AGENTS, help=f"Max concurrent agents (default: {MAX_AGENTS})")
    parser.add_argument("--model", default=None, help="LLM model for pi agents")
    parser.add_argument("--provider", default=None, help="LLM provider for pi agents (default: copilot)")
    parser.add_argument("--auto", type=int, default=0, metavar="N", help="Auto-start N agents with random units")
    args = parser.parse_args()

    if not shutil.which(PI_CMD):
        print(
            "pi (pi-coding-agent) not found in PATH.\n"
            "Install with: npm install -g @mariozechner/pi-coding-agent",
            file=sys.stderr,
        )
        return 1

    run_dashboard(
        version=args.version,
        category=args.category,
        max_agents=args.max_agents,
        model=args.model,
        provider=args.provider,
        auto_start=args.auto,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
