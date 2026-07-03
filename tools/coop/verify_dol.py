#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path

from dol import DolImage, decode_branch_target


def parse_int(value: int | str) -> int:
    if isinstance(value, int):
        return value
    return int(value, 0)


def elf_sections(readelf: Path, elf: Path) -> dict[str, tuple[int, int]]:
    proc = subprocess.run(
        [str(readelf), "-S", str(elf)],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    )
    sections: dict[str, tuple[int, int]] = {}
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 7 and parts[0].startswith("[") and parts[1].startswith("."):
            sections[parts[1]] = (int(parts[3], 16), int(parts[5], 16))
        elif len(parts) >= 8 and parts[0] == "[" and parts[2].startswith("."):
            sections[parts[2]] = (int(parts[4], 16), int(parts[6], 16))
    return sections


def elf_symbols(readelf: Path, elf: Path) -> dict[str, tuple[int, int]]:
    proc = subprocess.run(
        [str(readelf), "-s", str(elf)],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    )
    symbols: dict[str, tuple[int, int]] = {}
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 8 and parts[0].rstrip(":").isdigit():
            try:
                symbols[parts[7]] = (int(parts[1], 16), int(parts[2], 10))
            except ValueError:
                pass
    return symbols


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base", required=True, type=Path)
    parser.add_argument("--coop", required=True, type=Path)
    parser.add_argument("--elf", required=True, type=Path)
    parser.add_argument("--hooks", required=True, type=Path)
    parser.add_argument("--patch-report", required=True, type=Path)
    parser.add_argument("--readelf", required=True, type=Path)
    parser.add_argument("-o", "--out", required=True, type=Path)
    args = parser.parse_args()

    manifest = json.loads(args.hooks.read_text(encoding="utf-8"))
    report = json.loads(args.patch_report.read_text(encoding="utf-8"))
    base = DolImage.read(args.base)
    coop = DolImage.read(args.coop)
    sections = elf_sections(args.readelf, args.elf)
    symbols = elf_symbols(args.readelf, args.elf)

    if ".coop_text" not in sections or ".coop_data" not in sections:
        raise SystemExit("co-op ELF is missing .coop_text or .coop_data")
    coop_text_addr, coop_text_size = sections[".coop_text"]
    coop_data_addr, coop_data_size = sections[".coop_data"]
    if coop_text_size == 0 or coop_data_size == 0:
        raise SystemExit(".coop_text and .coop_data must be non-empty")
    if coop_text_addr + coop_text_size > coop_data_addr:
        raise SystemExit(".coop_text overlaps .coop_data")
    mailbox = symbols.get("gCoopMailbox")
    if mailbox is None:
        raise SystemExit("gCoopMailbox symbol missing")
    mailbox_addr, mailbox_size = mailbox
    if not (coop_data_addr <= mailbox_addr and mailbox_addr + mailbox_size <= coop_data_addr + coop_data_size):
        raise SystemExit("gCoopMailbox is not fully inside .coop_data")
    arena = symbols.get("__ArenaLo")
    if arena is None:
        raise SystemExit("__ArenaLo symbol missing")
    if arena[0] < coop_data_addr + coop_data_size:
        raise SystemExit("__ArenaLo does not follow the co-op reservation")

    retail_path = Path("orig") / "GCBE7D" / "sys" / "main.dol"
    if retail_path.exists():
        retail = DolImage.read(retail_path)
        for retail_section in retail.sections:
            matches = [
                section
                for section in base.sections
                if section.kind == retail_section.kind
                and section.address == retail_section.address
                and section.size == retail_section.size
            ]
            if not matches:
                raise SystemExit(
                    "original section changed or missing: "
                    f"{retail_section.kind}[{retail_section.index}] "
                    f"0x{retail_section.address:08X} size 0x{retail_section.size:X}"
                )
        if retail.bss_address != base.bss_address or retail.bss_size != base.bss_size:
            raise SystemExit("original BSS range changed")

    if len(base.data) > len(coop.data):
        raise SystemExit("patched DOL is smaller than the unpatched DOL")

    approved = {parse_int(hook["address"]) for hook in manifest["hooks"]}
    approved.update(parse_int(patch["address"]) for patch in manifest.get("word_patches", []))
    diffs: list[int] = []
    for section in base.sections:
        for offset in range(0, section.size, 4):
            addr = section.address + offset
            if base.read_u32(addr) != coop.read_u32(addr):
                diffs.append(addr)
    unexpected = [addr for addr in diffs if addr not in approved]
    if unexpected:
        raise SystemExit(
            "unexpected patched words: " + ", ".join(f"0x{addr:08X}" for addr in unexpected[:16])
        )
    missing = [addr for addr in approved if addr not in diffs]
    if missing:
        raise SystemExit(
            "approved words were not patched: " + ", ".join(f"0x{addr:08X}" for addr in missing)
        )

    reported_word_patches = {
        parse_int(item["address"]): parse_int(item["patched_opcode"])
        for item in report.get("word_patches", [])
    }
    for patch in manifest.get("word_patches", []):
        addr = parse_int(patch["address"])
        expected = parse_int(patch["patched_opcode"])
        if reported_word_patches.get(addr) != expected:
            raise SystemExit(f"word patch report mismatch for {patch['name']}")
        if coop.read_u32(addr) != expected:
            raise SystemExit(f"word patch value mismatch for {patch['name']}")

    report_targets = {item["name"]: parse_int(item["wrapper_target"]) for item in report["hooks"]}
    for hook in manifest["hooks"]:
        addr = parse_int(hook["address"])
        target = decode_branch_target(addr, coop.read_u32(addr))
        if target != report_targets[hook["name"]]:
            raise SystemExit(f"patched target mismatch for {hook['name']}")
        if not (coop_text_addr <= target < coop_text_addr + coop_text_size):
            raise SystemExit(f"hook {hook['name']} target is not inside .coop_text")

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text("ok\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
