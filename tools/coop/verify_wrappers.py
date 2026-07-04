#!/usr/bin/env python3
from __future__ import annotations

import argparse
import struct
import subprocess
from dataclasses import dataclass
from pathlib import Path

from dol import decode_branch_target


@dataclass(frozen=True)
class ElfSection:
    index: int
    name: str
    address: int
    offset: int
    size: int

    @property
    def end(self) -> int:
        return self.address + self.size


@dataclass(frozen=True)
class ElfSymbol:
    name: str
    address: int
    size: int
    section_index: int | None


def parse_section_index(value: str) -> int | None:
    try:
        return int(value)
    except ValueError:
        return None


def elf_sections(readelf: Path, elf: Path) -> dict[str, ElfSection]:
    proc = subprocess.run(
        [str(readelf), "-SW", str(elf)],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    )
    sections: dict[str, ElfSection] = {}
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 8 and parts[0] == "[" and parts[2].startswith("."):
            section = ElfSection(
                index=int(parts[1].rstrip("]")),
                name=parts[2],
                address=int(parts[4], 16),
                offset=int(parts[5], 16),
                size=int(parts[6], 16),
            )
            sections[section.name] = section
        elif len(parts) >= 7 and parts[0].startswith("[") and parts[1].startswith("."):
            section = ElfSection(
                index=int(parts[0].lstrip("[").rstrip("]")),
                name=parts[1],
                address=int(parts[3], 16),
                offset=int(parts[4], 16),
                size=int(parts[5], 16),
            )
            sections[section.name] = section
    return sections


def elf_symbols(readelf: Path, elf: Path) -> dict[str, ElfSymbol]:
    proc = subprocess.run(
        [str(readelf), "-sW", str(elf)],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    )
    symbols: dict[str, ElfSymbol] = {}
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 8 and parts[0].rstrip(":").isdigit():
            try:
                symbol = ElfSymbol(
                    name=parts[7],
                    address=int(parts[1], 16),
                    size=int(parts[2], 10),
                    section_index=parse_section_index(parts[6]),
                )
            except ValueError:
                continue
            symbols[symbol.name] = symbol
    return symbols


def symbol_extent(symbol: ElfSymbol, symbols: dict[str, ElfSymbol], section: ElfSection) -> int:
    if symbol.size > 0:
        return symbol.size
    next_addresses = [
        other.address
        for other in symbols.values()
        if other.section_index == symbol.section_index and other.address > symbol.address
    ]
    end = min(next_addresses) if next_addresses else section.end
    return end - symbol.address


def direct_bl_targets(data: bytes, section: ElfSection, symbol: ElfSymbol, size: int) -> list[int]:
    start = section.offset + (symbol.address - section.address)
    end = start + size
    targets: list[int] = []
    for offset in range(start, end, 4):
        instruction = struct.unpack_from(">I", data, offset)[0]
        if (instruction >> 26) == 18 and (instruction & 1) != 0:
            address = section.address + (offset - section.offset)
            targets.append(decode_branch_target(address, instruction))
    return targets


def symbol_name_for_target(symbols: dict[str, ElfSymbol], target: int) -> str:
    for symbol in symbols.values():
        if symbol.address == target:
            return symbol.name
    return f"0x{target:08X}"


def require_calls(wrapper: str, calls: list[str], expected: list[str]) -> None:
    if calls != expected:
        raise SystemExit(
            f"{wrapper} call sequence mismatch: got {calls!r}, expected {expected!r}"
        )
    if wrapper in calls:
        raise SystemExit(f"{wrapper} calls itself")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--elf", required=True, type=Path)
    parser.add_argument("--readelf", required=True, type=Path)
    parser.add_argument("-o", "--out", required=True, type=Path)
    args = parser.parse_args()

    sections = elf_sections(args.readelf, args.elf)
    symbols = elf_symbols(args.readelf, args.elf)
    data = args.elf.read_bytes()

    coop_text = sections.get(".coop_text")
    if coop_text is None:
        raise SystemExit("co-op ELF is missing .coop_text")

    required_symbols = [
        "CoopUpdatePlayerStatsWrapper",
        "CoopDrawCreaturesWrapper",
        "UpdatePlayerStats",
        "CoopFrameUpdate",
        "DrawCreatures",
        "CoopDrawRemotePlayer",
    ]
    missing = [name for name in required_symbols if name not in symbols]
    if missing:
        raise SystemExit("missing symbols: " + ", ".join(missing))

    expected_calls = {
        "CoopUpdatePlayerStatsWrapper": ["UpdatePlayerStats", "CoopFrameUpdate"],
        "CoopDrawCreaturesWrapper": ["DrawCreatures", "CoopDrawRemotePlayer"],
    }
    for wrapper, expected in expected_calls.items():
        symbol = symbols[wrapper]
        if symbol.section_index != coop_text.index:
            raise SystemExit(f"{wrapper} is not in .coop_text")
        size = symbol_extent(symbol, symbols, coop_text)
        calls = [
            symbol_name_for_target(symbols, target)
            for target in direct_bl_targets(data, coop_text, symbol, size)
        ]
        require_calls(wrapper, calls, expected)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text("ok\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
