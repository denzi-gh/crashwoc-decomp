#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


ALLOWED_ALLOC = {".coop_text", ".coop_data"}


def allocated_sections(readelf: Path, obj: Path) -> list[str]:
    proc = subprocess.run(
        [str(readelf), "-S", str(obj)],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    )
    sections: list[str] = []
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 8 and parts[0] == "[" and parts[2].startswith("."):
            name = parts[2]
            flags = parts[7]
        elif len(parts) >= 7 and parts[0].startswith("[") and parts[1].startswith("."):
            name = parts[1]
            flags = parts[6]
        else:
            continue
        if "A" in flags:
            sections.append(name)
    return sections


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("object", type=Path)
    parser.add_argument("--readelf", required=True, type=Path)
    parser.add_argument("-o", "--out", required=True, type=Path)
    args = parser.parse_args()

    bad = [name for name in allocated_sections(args.readelf, args.object) if name not in ALLOWED_ALLOC]
    if bad:
        print(f"unexpected allocated co-op object sections: {', '.join(bad)}")
        return 1
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text("ok\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
