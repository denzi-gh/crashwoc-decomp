#!/usr/bin/env python3
"""Generate a local HTML dashboard for pi_orchestrator summary JSON files."""
from __future__ import annotations

import json
import webbrowser
from datetime import datetime
from html import escape
from pathlib import Path
from typing import Any


def _as_int(value: Any, default: int = 0) -> int:
    try:
        return int(value)
    except (TypeError, ValueError):
        return default


def _as_float(value: Any, default: float = 0.0) -> float:
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def _pct(numerator: int, denominator: int) -> float:
    if denominator <= 0:
        return 0.0
    return max(0.0, min(100.0, (numerator / denominator) * 100.0))


def _format_seconds(seconds: Any) -> str:
    total_seconds = max(0, int(round(_as_float(seconds))))
    hours, remainder = divmod(total_seconds, 3600)
    minutes, secs = divmod(remainder, 60)
    if hours:
        return f"{hours}h {minutes:02d}m {secs:02d}s"
    if minutes:
        return f"{minutes}m {secs:02d}s"
    return f"{secs}s"


def _format_timestamp(value: Any) -> str:
    raw = str(value or "").strip()
    if not raw:
        return "-"
    try:
        stamp = datetime.fromisoformat(raw)
    except ValueError:
        return escape(raw)
    return stamp.strftime("%Y-%m-%d %H:%M:%S")


def _output_path_for(summary_path: Path) -> Path:
    if summary_path.suffix:
        return summary_path.with_name(f"{summary_path.stem}.dashboard.html")
    return summary_path.with_name(f"{summary_path.name}.dashboard.html")


def _resolve_summary_path(summary_path: str | Path) -> Path:
    candidate = Path(summary_path).expanduser().resolve()
    if candidate.is_dir():
        json_files = sorted(
            candidate.glob("*.json"),
            key=lambda path: path.stat().st_mtime,
            reverse=True,
        )
        if not json_files:
            raise FileNotFoundError(f"No summary JSON files found in: {candidate}")
        return json_files[0]
    return candidate


def _bar_html(percent: float, label: str, modifier: str = "") -> str:
    class_name = "meter"
    if modifier:
        class_name += f" {modifier}"
    width = f"{percent:.1f}%"
    value = f"{percent:.1f}%"
    return (
        f'<div class="{class_name}">'
        '<div class="meter-meta">'
        f'<span class="meter-label">{escape(label)}</span>'
        f'<span class="meter-value">{escape(value)}</span>'
        "</div>"
        '<div class="meter-track">'
        f'<span class="meter-fill" style="width:{width}"></span>'
        "</div>"
        f"</div>"
    )


def _ring_html(percent: float, value: str, label: str, tone: str = "primary") -> str:
    percent = max(0.0, min(100.0, percent))
    return (
        f'<div class="ring ring-{tone}" style="--ring-pct:{percent:.1f}%;">'
        '<div class="ring-core">'
        f'<div class="ring-value">{escape(value)}</div>'
        f'<div class="ring-label">{escape(label)}</div>'
        "</div>"
        "</div>"
    )


def _completed_rows(completed_units: list[dict[str, Any]]) -> str:
    if not completed_units:
        return (
            '<div class="empty-state">'
            "No completed units were recorded in this session."
            "</div>"
        )

    ordered = sorted(
        completed_units,
        key=lambda item: (
            -_as_int(item.get("matched_during_session")),
            _as_float(item.get("duration_seconds")),
            str(item.get("source_path", "")),
        ),
    )

    rows: list[str] = []
    for item in ordered:
        source_path = escape(str(item.get("source_path", "-")))
        total = _as_int(item.get("total"))
        gained = _as_int(item.get("matched_during_session"))
        gain_pct = _pct(gained, total)
        duration = _format_seconds(item.get("duration_seconds"))
        rows.append(
            "<tr>"
            f"<td><code>{source_path}</code></td>"
            f"<td>{gained}</td>"
            f"<td>{total}</td>"
            f"<td>{_bar_html(gain_pct, 'of unit matched this session', 'good')}</td>"
            f"<td>{escape(duration)}</td>"
            "</tr>"
        )
    return (
        "<table>"
        "<thead><tr><th>Unit</th><th>Gained</th><th>Total</th><th>Impact</th><th>Time</th></tr></thead>"
        "<tbody>"
        + "".join(rows)
        + "</tbody></table>"
    )


def _blocked_rows(blocked_units: list[dict[str, Any]]) -> str:
    if not blocked_units:
        return (
            '<div class="empty-state">'
            "No blocked units were recorded in this session."
            "</div>"
        )

    ordered = sorted(
        blocked_units,
        key=lambda item: (
            -(_as_int(item.get("total")) - _as_int(item.get("matched"))),
            str(item.get("source_path", "")),
        ),
    )

    rows: list[str] = []
    for item in ordered:
        source_path = escape(str(item.get("source_path", "-")))
        total = _as_int(item.get("total"))
        matched = _as_int(item.get("matched"))
        blocked_functions = [
            escape(str(name)) for name in item.get("blocked_functions", []) if str(name).strip()
        ]
        match_pct = _pct(matched, total)
        chips = "".join(f'<span class="chip">{name}</span>' for name in blocked_functions) or "-"
        rows.append(
            "<tr>"
            f"<td><code>{source_path}</code></td>"
            f"<td>{matched}/{total}</td>"
            f"<td>{_bar_html(match_pct, 'matched', 'warn')}</td>"
            f"<td><div class=\"chip-wrap\">{chips}</div></td>"
            "</tr>"
        )
    return (
        "<table>"
        "<thead><tr><th>Unit</th><th>Matched</th><th>Progress</th><th>Blocked Functions</th></tr></thead>"
        "<tbody>"
        + "".join(rows)
        + "</tbody></table>"
    )


def build_summary_dashboard_html(summary_path: Path, summary_data: dict[str, Any]) -> str:
    start_matched = _as_int(summary_data.get("functions_matched_start"))
    end_matched = _as_int(summary_data.get("functions_matched_end"))
    total_functions = _as_int(summary_data.get("total_functions"))
    gained = _as_int(summary_data.get("functions_gained"), end_matched - start_matched)
    duration = _format_seconds(summary_data.get("duration_seconds"))
    completed_units = list(summary_data.get("completed_units", []))
    blocked_units = list(summary_data.get("blocked_units", []))
    start_pct = _pct(start_matched, total_functions)
    end_pct = _pct(end_matched, total_functions)

    summary_name = escape(summary_path.name)
    summary_location = escape(str(summary_path))
    started_at = _format_timestamp(summary_data.get("session_start"))
    ended_at = _format_timestamp(summary_data.get("session_end"))
    completed_ratio = _pct(len(completed_units), max(len(completed_units) + len(blocked_units), 1))
    gain_pct = _pct(gained, max(total_functions, 1))

    return f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="color-scheme" content="dark">
  <title>PI Orchestrator Dashboard</title>
  <style>
    :root {{
      --bg: #0b1020;
      --bg-2: #141c31;
      --surface: rgba(18, 24, 42, 0.78);
      --surface-strong: rgba(25, 33, 58, 0.92);
      --surface-soft: rgba(16, 22, 39, 0.68);
      --ink: #f3f3fb;
      --muted: #aeb5d6;
      --line: rgba(179, 189, 255, 0.15);
      --line-strong: rgba(214, 220, 255, 0.22);
      --accent: #f5b7d2;
      --accent-2: #c8b6ff;
      --accent-3: #9adfd6;
      --good: #a9e7cf;
      --warn: #ffd6a5;
      --danger: #ffb7c8;
      --shadow: 0 30px 80px rgba(3, 6, 18, 0.55);
      --radius: 26px;
    }}

    * {{
      box-sizing: border-box;
    }}

    html {{
      color-scheme: dark;
    }}

    body {{
      margin: 0;
      font-family: "Aptos", "Segoe UI Variable Display", "Trebuchet MS", sans-serif;
      color: var(--ink);
      background:
        radial-gradient(circle at 12% 12%, rgba(245, 183, 210, 0.18), transparent 0 23%),
        radial-gradient(circle at 82% 10%, rgba(200, 182, 255, 0.18), transparent 0 20%),
        radial-gradient(circle at 80% 72%, rgba(154, 223, 214, 0.16), transparent 0 18%),
        linear-gradient(180deg, #0b1020 0%, #10152a 45%, #0d1325 100%);
      min-height: 100vh;
    }}

    body::before,
    body::after {{
      content: "";
      position: fixed;
      inset: auto;
      width: 34rem;
      height: 34rem;
      border-radius: 50%;
      filter: blur(90px);
      pointer-events: none;
      opacity: 0.18;
      z-index: 0;
    }}

    body::before {{
      top: -10rem;
      right: -6rem;
      background: #f5b7d2;
    }}

    body::after {{
      left: -12rem;
      bottom: -16rem;
      background: #9adfd6;
    }}

    .shell {{
      position: relative;
      z-index: 1;
      max-width: 1440px;
      margin: 0 auto;
      padding: 34px 20px 64px;
    }}

    .hero {{
      padding: 32px;
      border: 1px solid var(--line);
      border-radius: calc(var(--radius) + 8px);
      background:
        linear-gradient(160deg, rgba(255, 255, 255, 0.06), rgba(255, 255, 255, 0.01)),
        linear-gradient(135deg, rgba(17, 23, 41, 0.92), rgba(16, 20, 37, 0.82));
      box-shadow: var(--shadow);
      backdrop-filter: blur(26px);
    }}

    .hero-grid {{
      display: grid;
      grid-template-columns: minmax(0, 1.2fr) minmax(280px, 0.8fr);
      gap: 24px;
      align-items: stretch;
    }}

    .eyebrow {{
      margin: 0 0 8px;
      font-size: 12px;
      letter-spacing: 0.18em;
      text-transform: uppercase;
      color: var(--accent);
      font-weight: 700;
    }}

    h1 {{
      margin: 0;
      font-family: "Aptos Display", "Segoe UI Variable Display", "Aptos", sans-serif;
      font-size: clamp(36px, 5vw, 64px);
      line-height: 0.92;
      letter-spacing: -0.04em;
      max-width: 10ch;
    }}

    .subtitle {{
      margin: 14px 0 0;
      max-width: 72ch;
      color: var(--muted);
      font-size: 15px;
      line-height: 1.7;
    }}

    .meta {{
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-top: 18px;
    }}

    .pill {{
      padding: 8px 12px;
      border-radius: 999px;
      border: 1px solid var(--line-strong);
      background: rgba(255, 255, 255, 0.04);
      color: var(--muted);
      font-size: 13px;
      backdrop-filter: blur(12px);
    }}

    .hero-visual {{
      position: relative;
      overflow: hidden;
      border-radius: calc(var(--radius) - 2px);
      border: 1px solid var(--line);
      background:
        radial-gradient(circle at 30% 22%, rgba(245, 183, 210, 0.18), transparent 0 22%),
        radial-gradient(circle at 72% 24%, rgba(200, 182, 255, 0.18), transparent 0 24%),
        linear-gradient(180deg, rgba(20, 27, 49, 0.96), rgba(12, 17, 30, 0.96));
      min-height: 100%;
      padding: 22px;
    }}

    .hero-visual::before {{
      content: "";
      position: absolute;
      inset: 1rem auto auto 1rem;
      width: 7rem;
      height: 7rem;
      border-radius: 50%;
      background: rgba(154, 223, 214, 0.12);
      filter: blur(16px);
    }}

    .hero-visual::after {{
      content: "";
      position: absolute;
      right: 1.5rem;
      bottom: 1.5rem;
      width: 9rem;
      height: 9rem;
      border-radius: 50%;
      background: rgba(200, 182, 255, 0.12);
      filter: blur(18px);
    }}

    .visual-stack {{
      position: relative;
      z-index: 1;
      display: grid;
      gap: 18px;
      height: 100%;
    }}

    .visual-top {{
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 16px;
    }}

    .visual-caption {{
      max-width: 14rem;
    }}

    .visual-kicker {{
      color: var(--accent-3);
      font-size: 12px;
      letter-spacing: 0.12em;
      text-transform: uppercase;
      font-weight: 700;
    }}

    .visual-title {{
      margin-top: 8px;
      font-size: 28px;
      line-height: 1;
      letter-spacing: -0.04em;
      font-weight: 800;
    }}

    .visual-note {{
      margin-top: 8px;
      font-size: 13px;
      color: var(--muted);
      line-height: 1.6;
    }}

    .grid {{
      display: grid;
      gap: 18px;
      margin-top: 22px;
    }}

    .stats {{
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    }}

    .main {{
      grid-template-columns: minmax(0, 1.2fr) minmax(0, 1fr);
      align-items: start;
    }}

    .card {{
      background: var(--surface);
      border: 1px solid var(--line);
      border-radius: var(--radius);
      box-shadow: var(--shadow);
      overflow: hidden;
      backdrop-filter: blur(18px);
    }}

    .card-inner {{
      padding: 22px;
    }}

    .stat-label {{
      color: var(--accent-3);
      font-size: 12px;
      font-weight: 700;
      letter-spacing: 0.12em;
      text-transform: uppercase;
    }}

    .stat-value {{
      margin-top: 8px;
      font-size: 36px;
      line-height: 1;
      font-weight: 800;
      letter-spacing: -0.04em;
    }}

    .stat-note {{
      margin-top: 8px;
      color: var(--muted);
      font-size: 14px;
    }}

    .section-title {{
      margin: 0;
      font-size: 24px;
      letter-spacing: -0.03em;
    }}

    .section-kicker {{
      margin: 4px 0 0;
      color: var(--muted);
      font-size: 14px;
    }}

    .meter {{
      display: grid;
      gap: 10px;
      min-width: 170px;
    }}

    .meter-meta {{
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 10px;
    }}

    .meter-label {{
      color: var(--muted);
      font-size: 12px;
      line-height: 1.45;
      font-weight: 650;
    }}

    .meter-value {{
      flex: 0 0 auto;
      min-width: 4.9rem;
      padding: 4px 10px;
      border-radius: 999px;
      border: 1px solid rgba(255, 255, 255, 0.08);
      background: rgba(255, 255, 255, 0.06);
      color: #f8dbff;
      font-size: 11px;
      line-height: 1;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      text-align: center;
      font-weight: 800;
      box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.05);
    }}

    .meter.good .meter-value {{
      color: var(--good);
    }}

    .meter.warn .meter-value {{
      color: var(--warn);
    }}

    .meter-track {{
      position: relative;
      height: 14px;
      overflow: hidden;
      border-radius: 999px;
      background:
        linear-gradient(180deg, rgba(255,255,255,0.06), rgba(255,255,255,0.02));
      border: 1px solid rgba(255, 255, 255, 0.08);
      box-shadow:
        inset 0 1px 2px rgba(0, 0, 0, 0.35),
        inset 0 -1px 0 rgba(255, 255, 255, 0.03);
    }}

    .meter-track::before {{
      content: "";
      position: absolute;
      inset: 0;
      background: repeating-linear-gradient(
        90deg,
        rgba(255, 255, 255, 0.09) 0 1px,
        transparent 1px 22px
      );
      opacity: 0.4;
      pointer-events: none;
    }}

    .meter-fill {{
      position: absolute;
      inset: 0 auto 0 0;
      display: block;
      background: linear-gradient(90deg, var(--accent) 0%, var(--accent-2) 100%);
      border-radius: inherit;
      box-shadow:
        0 0 24px rgba(200, 182, 255, 0.26),
        inset 0 1px 0 rgba(255, 255, 255, 0.18);
      transition: width 220ms ease;
    }}

    .meter-fill::after {{
      content: "";
      position: absolute;
      inset: 0;
      background: linear-gradient(
        110deg,
        rgba(255, 255, 255, 0) 0%,
        rgba(255, 255, 255, 0.18) 42%,
        rgba(255, 255, 255, 0) 70%
      );
      transform: translateX(0);
      opacity: 0.35;
      pointer-events: none;
    }}

    .meter.good .meter-fill {{
      background: linear-gradient(90deg, var(--accent-3) 0%, var(--good) 100%);
      box-shadow:
        0 0 24px rgba(169, 231, 207, 0.18),
        inset 0 1px 0 rgba(255, 255, 255, 0.16);
    }}

    .meter.warn .meter-fill {{
      background: linear-gradient(90deg, var(--warn) 0%, #f5b7d2 100%);
      box-shadow:
        0 0 24px rgba(255, 214, 165, 0.16),
        inset 0 1px 0 rgba(255, 255, 255, 0.14);
    }}

    .ring {{
      --ring-pct: 0%;
      position: relative;
      width: 170px;
      aspect-ratio: 1;
      border-radius: 50%;
      display: grid;
      place-items: center;
      background:
        radial-gradient(closest-side, rgba(11, 16, 32, 0.96) 73%, transparent 74% 100%),
        conic-gradient(var(--ring-tone) var(--ring-pct), rgba(255, 255, 255, 0.08) 0);
      box-shadow:
        inset 0 0 0 1px rgba(255, 255, 255, 0.06),
        0 18px 40px rgba(4, 8, 22, 0.38);
    }}

    .ring-primary {{
      --ring-tone: linear-gradient(135deg, var(--accent), var(--accent-2));
      background:
        radial-gradient(closest-side, rgba(11, 16, 32, 0.96) 73%, transparent 74% 100%),
        conic-gradient(var(--accent) var(--ring-pct), rgba(255, 255, 255, 0.08) 0);
    }}

    .ring-good {{
      --ring-tone: linear-gradient(135deg, var(--accent-3), var(--good));
      background:
        radial-gradient(closest-side, rgba(11, 16, 32, 0.96) 73%, transparent 74% 100%),
        conic-gradient(var(--accent-3) var(--ring-pct), rgba(255, 255, 255, 0.08) 0);
    }}

    .ring-warn {{
      --ring-tone: linear-gradient(135deg, var(--warn), var(--danger));
      background:
        radial-gradient(closest-side, rgba(11, 16, 32, 0.96) 73%, transparent 74% 100%),
        conic-gradient(var(--warn) var(--ring-pct), rgba(255, 255, 255, 0.08) 0);
    }}

    .ring-core {{
      display: grid;
      gap: 6px;
      text-align: center;
      padding: 18px;
    }}

    .ring-value {{
      font-size: 34px;
      line-height: 1;
      font-weight: 800;
      letter-spacing: -0.04em;
    }}

    .ring-label {{
      color: var(--muted);
      font-size: 13px;
      line-height: 1.4;
    }}

    table {{
      width: 100%;
      border-collapse: collapse;
      background: transparent;
    }}

    th,
    td {{
      padding: 14px 16px;
      text-align: left;
      vertical-align: top;
      border-top: 1px solid rgba(255, 255, 255, 0.06);
      font-size: 14px;
    }}

    thead th {{
      border-top: 0;
      color: #cdd2ef;
      font-size: 12px;
      letter-spacing: 0.12em;
      text-transform: uppercase;
      background: rgba(255, 255, 255, 0.03);
    }}

    tbody tr {{
      transition: background 140ms ease, transform 140ms ease;
    }}

    tbody tr:hover {{
      background: rgba(255, 255, 255, 0.03);
    }}

    code {{
      font-family: "Cascadia Code", "Consolas", monospace;
      font-size: 12px;
      color: #f4d7f4;
      white-space: normal;
      word-break: break-word;
    }}

    .chip-wrap {{
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
    }}

    .chip {{
      display: inline-flex;
      align-items: center;
      padding: 6px 10px;
      border-radius: 999px;
      background: rgba(255, 255, 255, 0.05);
      border: 1px solid rgba(255, 255, 255, 0.08);
      color: #f1e8ff;
      font-size: 12px;
      backdrop-filter: blur(8px);
    }}

    .empty-state {{
      padding: 24px 20px 28px;
      color: var(--muted);
      font-size: 15px;
    }}

    .timeline {{
      display: grid;
      gap: 14px;
    }}

    .timeline-item {{
      padding: 16px 18px;
      border-radius: 18px;
      background: linear-gradient(180deg, rgba(255,255,255,0.05), rgba(255,255,255,0.02));
      border: 1px solid rgba(255, 255, 255, 0.08);
    }}

    .timeline-label {{
      color: var(--accent);
      font-size: 12px;
      font-weight: 700;
      letter-spacing: 0.12em;
      text-transform: uppercase;
    }}

    .timeline-value {{
      margin-top: 6px;
      font-size: 20px;
      font-weight: 700;
      letter-spacing: -0.03em;
    }}

    .footer-note {{
      margin-top: 18px;
      color: var(--muted);
      font-size: 13px;
    }}

    .subgrid {{
      display: grid;
      gap: 18px;
    }}

    .insight-grid {{
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 14px;
    }}

    .mini-stat {{
      padding: 16px 18px;
      border-radius: 18px;
      background: rgba(255, 255, 255, 0.04);
      border: 1px solid rgba(255, 255, 255, 0.08);
    }}

    .mini-stat-label {{
      color: var(--muted);
      font-size: 12px;
      letter-spacing: 0.1em;
      text-transform: uppercase;
      font-weight: 700;
    }}

    .mini-stat-value {{
      margin-top: 8px;
      font-size: 24px;
      font-weight: 800;
      letter-spacing: -0.03em;
    }}

    @media (max-width: 980px) {{
      .hero-grid,
      .main {{
        grid-template-columns: 1fr;
      }}

      .visual-top,
      .insight-grid {{
        grid-template-columns: 1fr;
      }}

      .visual-top {{
        flex-direction: column;
        align-items: flex-start;
      }}
    }}

    @media (max-width: 700px) {{
      .shell {{
        padding-inline: 14px;
      }}

      .hero,
      .card-inner {{
        padding: 20px;
      }}

      h1 {{
        font-size: 34px;
      }}

      .stat-value {{
        font-size: 30px;
      }}

      .ring {{
        width: 140px;
      }}
    }}
  </style>
</head>
<body>
  <div class="shell">
    <section class="hero">
      <div class="hero-grid">
        <div class="hero-copy">
          <p class="eyebrow">Pi Orchestrator Session</p>
          <h1>Summary Dashboard</h1>
          <p class="subtitle">
            Visual summary generated from <code>{summary_name}</code>. The layout is tuned for a
            modern dark dashboard feel: minimal chrome, pastel accents, and fast visual scanning of
            progress, wins, and blockers.
          </p>
          <div class="meta">
            <span class="pill">Source: <code>{summary_location}</code></span>
            <span class="pill">Started: {started_at}</span>
            <span class="pill">Ended: {ended_at}</span>
            <span class="pill">Duration: {escape(duration)}</span>
          </div>
        </div>
        <aside class="hero-visual">
          <div class="visual-stack">
            <div class="visual-top">
              {_ring_html(end_pct, f"{end_pct:.1f}%", "overall matched", "primary")}
              <div class="visual-caption">
                <div class="visual-kicker">Session Pulse</div>
                <div class="visual-title">+{gained}</div>
                <div class="visual-note">
                  Fresh matches recorded in this run. Completed and blocked counts are shown below
                  so the next pass is easy to prioritize.
                </div>
              </div>
            </div>
            <div class="insight-grid">
              <div class="mini-stat">
                <div class="mini-stat-label">Completed Share</div>
                <div class="mini-stat-value">{completed_ratio:.1f}%</div>
              </div>
              <div class="mini-stat">
                <div class="mini-stat-label">Gain vs Project</div>
                <div class="mini-stat-value">{gain_pct:.2f}%</div>
              </div>
            </div>
          </div>
        </aside>
      </div>
    </section>

    <section class="grid stats">
      <article class="card"><div class="card-inner">
        <div class="stat-label">Functions Gained</div>
        <div class="stat-value">+{gained}</div>
        <div class="stat-note">Matched during this orchestrator session.</div>
      </div></article>
      <article class="card"><div class="card-inner">
        <div class="stat-label">Project Progress</div>
        <div class="stat-value">{end_matched}/{total_functions}</div>
        <div class="stat-note">Ended at {end_pct:.2f}% overall match coverage.</div>
      </div></article>
      <article class="card"><div class="card-inner">
        <div class="stat-label">Completed Units</div>
        <div class="stat-value">{len(completed_units)}</div>
        <div class="stat-note">Units that reached completion during the session.</div>
      </div></article>
      <article class="card"><div class="card-inner">
        <div class="stat-label">Blocked Units</div>
        <div class="stat-value">{len(blocked_units)}</div>
        <div class="stat-note">Units that still had unmatched work when the run stopped.</div>
      </div></article>
    </section>

    <section class="grid main">
      <article class="card">
        <div class="card-inner">
          <h2 class="section-title">Completed Units</h2>
          <p class="section-kicker">Sorted by how many functions were gained in this session.</p>
        </div>
        {_completed_rows(completed_units)}
      </article>

      <div class="subgrid">
        <article class="card">
          <div class="card-inner">
            <h2 class="section-title">Session Arc</h2>
            <p class="section-kicker">Start and end checkpoints for the same run.</p>
            <div class="timeline">
              <div class="timeline-item">
                <div class="timeline-label">Starting Progress</div>
                <div class="timeline-value">{start_matched}/{total_functions}</div>
                {_bar_html(start_pct, "matched at session start")}
              </div>
              <div class="timeline-item">
                <div class="timeline-label">Ending Progress</div>
                <div class="timeline-value">{end_matched}/{total_functions}</div>
                {_bar_html(end_pct, "matched at session end", "good")}
              </div>
              <div class="timeline-item">
                <div class="timeline-label">Net Gain</div>
                <div class="timeline-value">+{gained} functions</div>
                <div class="footer-note">
                  Summary file generated by the orchestrator at the end of the run.
                </div>
              </div>
            </div>
          </div>
        </article>

        <article class="card">
          <div class="card-inner">
            <h2 class="section-title">Outcome Snapshot</h2>
            <p class="section-kicker">Quick visual pulse for the current state.</p>
            <div class="visual-top">
              {_ring_html(_pct(len(blocked_units), max(len(completed_units) + len(blocked_units), 1)), str(len(blocked_units)), "blocked units", "warn")}
              <div class="visual-caption">
                <div class="visual-kicker">Next Iteration</div>
                <div class="visual-title">{len(completed_units)} done / {len(blocked_units)} blocked</div>
                <div class="visual-note">
                  Use the blocked section below as the immediate backlog for the next orchestrator
                  run or manual follow-up pass.
                </div>
              </div>
            </div>
          </div>
        </article>
      </div>
    </section>

    <section class="grid">
      <article class="card">
        <div class="card-inner">
          <h2 class="section-title">Blocked Units</h2>
          <p class="section-kicker">Units that still have unresolved functions after the session.</p>
        </div>
        {_blocked_rows(blocked_units)}
      </article>
    </section>
  </div>
</body>
</html>
"""


def write_summary_dashboard(summary_path: str | Path, output_path: str | Path | None = None) -> Path:
    summary_file = _resolve_summary_path(summary_path)
    if not summary_file.is_file():
        raise FileNotFoundError(f"Summary JSON not found: {summary_file}")

    with summary_file.open("r", encoding="utf-8") as handle:
        summary_data = json.load(handle)

    target_path = Path(output_path).expanduser().resolve() if output_path else _output_path_for(summary_file)
    target_path.parent.mkdir(parents=True, exist_ok=True)
    target_path.write_text(
        build_summary_dashboard_html(summary_file, summary_data),
        encoding="utf-8",
    )
    return target_path


def open_summary_dashboard(summary_path: str | Path, output_path: str | Path | None = None) -> Path:
    target_path = write_summary_dashboard(summary_path, output_path=output_path)
    try:
        webbrowser.open(target_path.resolve().as_uri())
    except Exception:
        pass
    return target_path
