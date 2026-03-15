#!/usr/bin/env python3
"""
Optional build lock wrapper for concurrent ninja invocations.

Usage:
    python tools/pi_build_lock.py ninja build/GCBE7D/src/camera.o

Acquires a file lock before running the command, ensuring only one
ninja build runs at a time. Only needed if concurrent ninja causes
issues on Windows/MSYS2.

Requires: pip install filelock
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

try:
    from filelock import FileLock
except ImportError:
    print(
        "Error: filelock package not installed.\n"
        "Install with: pip install filelock\n"
        "Or run ninja directly without the lock wrapper.",
        file=sys.stderr,
    )
    sys.exit(1)

ROOT = Path(__file__).resolve().parent.parent
LOCK_PATH = ROOT / "tmp" / ".ninja_lock"


def main() -> int:
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <command> [args...]", file=sys.stderr)
        return 1

    LOCK_PATH.parent.mkdir(parents=True, exist_ok=True)

    with FileLock(str(LOCK_PATH), timeout=300):
        return subprocess.call(sys.argv[1:], cwd=str(ROOT))


if __name__ == "__main__":
    raise SystemExit(main())
