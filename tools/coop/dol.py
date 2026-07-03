from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class DolSection:
    index: int
    kind: str
    offset: int
    address: int
    size: int

    @property
    def end(self) -> int:
        return self.address + self.size


class DolImage:
    def __init__(self, data: bytes):
        if len(data) < 0x100:
            raise ValueError("DOL is too small")
        self.data = bytearray(data)
        self.sections = self._read_sections()
        self.bss_address, self.bss_size, self.entry = struct.unpack_from(">3I", data, 0xD8)

    @classmethod
    def read(cls, path: str | Path) -> "DolImage":
        return cls(Path(path).read_bytes())

    def write(self, path: str | Path) -> None:
        Path(path).parent.mkdir(parents=True, exist_ok=True)
        Path(path).write_bytes(self.data)

    def _read_sections(self) -> list[DolSection]:
        text_offsets = struct.unpack_from(">7I", self.data, 0x00)
        data_offsets = struct.unpack_from(">11I", self.data, 0x1C)
        text_addrs = struct.unpack_from(">7I", self.data, 0x48)
        data_addrs = struct.unpack_from(">11I", self.data, 0x64)
        text_sizes = struct.unpack_from(">7I", self.data, 0x90)
        data_sizes = struct.unpack_from(">11I", self.data, 0xAC)
        sections: list[DolSection] = []
        for i, (off, addr, size) in enumerate(zip(text_offsets, text_addrs, text_sizes)):
            if size:
                sections.append(DolSection(i, "text", off, addr, size))
        for i, (off, addr, size) in enumerate(zip(data_offsets, data_addrs, data_sizes)):
            if size:
                sections.append(DolSection(i, "data", off, addr, size))
        return sections

    def section_for_address(self, address: int, size: int = 1) -> DolSection:
        for section in self.sections:
            if section.address <= address and address + size <= section.end:
                return section
        raise ValueError(f"address 0x{address:08X} is outside loaded DOL sections")

    def address_to_offset(self, address: int, size: int = 1) -> int:
        section = self.section_for_address(address, size)
        return section.offset + (address - section.address)

    def read_u32(self, address: int) -> int:
        if address & 3:
            raise ValueError(f"address 0x{address:08X} is not 4-byte aligned")
        off = self.address_to_offset(address, 4)
        return struct.unpack_from(">I", self.data, off)[0]

    def write_u32(self, address: int, value: int) -> None:
        if address & 3:
            raise ValueError(f"address 0x{address:08X} is not 4-byte aligned")
        off = self.address_to_offset(address, 4)
        struct.pack_into(">I", self.data, off, value & 0xFFFFFFFF)


def sign_extend(value: int, bits: int) -> int:
    sign = 1 << (bits - 1)
    return (value ^ sign) - sign


def decode_branch_target(address: int, instruction: int) -> int:
    if (instruction >> 26) != 18:
        raise ValueError(f"instruction 0x{instruction:08X} is not a PowerPC branch")
    li = instruction & 0x03FFFFFC
    disp = sign_extend(li, 26)
    if instruction & 2:
        return disp & 0xFFFFFFFF
    return (address + disp) & 0xFFFFFFFF


def encode_bl(address: int, target: int) -> int:
    if (address & 3) or (target & 3):
        raise ValueError("branch source and target must be 4-byte aligned")
    disp = target - address
    if disp < -0x02000000 or disp > 0x01FFFFFC:
        raise ValueError(
            f"branch target 0x{target:08X} is out of range from 0x{address:08X}"
        )
    return 0x48000001 | (disp & 0x03FFFFFC)
