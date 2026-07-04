from __future__ import annotations

import struct
from dataclasses import dataclass
from typing import Any

from . import protocol_generated as proto


MAILBOX_ADDRESS = 0x803F6000
DIFFERENT_LOCATION_SENTINEL = -0x40000000


class MailboxReadinessError(RuntimeError):
    pass


class MemoryAdapter:
    def hook(self) -> None:
        raise NotImplementedError

    def is_hooked(self) -> bool:
        raise NotImplementedError

    def read(self, address: int, size: int) -> bytes:
        raise NotImplementedError

    def write(self, address: int, data: bytes) -> None:
        raise NotImplementedError


class DolphinMemoryAdapter(MemoryAdapter):
    def __init__(self) -> None:
        import dolphin_memory_engine as dme

        self._dme = dme

    def hook(self) -> None:
        self._dme.hook()

    def is_hooked(self) -> bool:
        return bool(self._dme.is_hooked())

    def read(self, address: int, size: int) -> bytes:
        return self._dme.read_bytes(address, size)

    def write(self, address: int, data: bytes) -> None:
        self._dme.write_bytes(address, data)


class FakeMemoryAdapter(MemoryAdapter):
    def __init__(self, size: int = 0x2000, base: int = MAILBOX_ADDRESS) -> None:
        self.base = base
        self.data = bytearray(size)
        self.hooked = True

    def hook(self) -> None:
        self.hooked = True

    def is_hooked(self) -> bool:
        return self.hooked

    def read(self, address: int, size: int) -> bytes:
        off = address - self.base
        if off < 0 or off + size > len(self.data):
            raise RuntimeError(f"fake read out of range: 0x{address:08X}")
        return bytes(self.data[off : off + size])

    def write(self, address: int, data: bytes) -> None:
        off = address - self.base
        if off < 0 or off + len(data) > len(self.data):
            raise RuntimeError(f"fake write out of range: 0x{address:08X}")
        self.data[off : off + len(data)] = data


@dataclass
class MailboxHeader:
    magic: int
    abi_version: int
    struct_size: int
    build_id: int
    capabilities: int
    game_heartbeat: int
    bridge_heartbeat: int
    local_seq: int
    inbound_seq: int
    last_applied_progress_revision: int
    status_flags: int


@dataclass
class CoopDebugState:
    magic: int
    calls: int
    reason: int
    draws: int
    hides: int
    local_level: int
    inbound_level: int
    same_location: int


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def put_u32(data: bytearray, offset: int, value: int) -> None:
    struct.pack_into(">I", data, offset, value & 0xFFFFFFFF)


def read_mailbox(adapter: MemoryAdapter, address: int = MAILBOX_ADDRESS) -> bytes:
    return adapter.read(address, proto.MAILBOX_SIZE)


def read_header(adapter: MemoryAdapter, address: int = MAILBOX_ADDRESS) -> MailboxHeader:
    data = read_mailbox(adapter, address)
    mailbox = proto.OFFSETS["CoopMailbox"]
    return MailboxHeader(
        magic=u32(data, mailbox["magic"]),
        abi_version=struct.unpack_from(">H", data, mailbox["abi_version"])[0],
        struct_size=struct.unpack_from(">H", data, mailbox["struct_size"])[0],
        build_id=u32(data, mailbox["build_id"]),
        capabilities=u32(data, mailbox["capabilities"]),
        game_heartbeat=u32(data, mailbox["game_heartbeat"]),
        bridge_heartbeat=u32(data, mailbox["bridge_heartbeat"]),
        local_seq=u32(data, mailbox["local_seq"]),
        inbound_seq=u32(data, mailbox["inbound_seq"]),
        last_applied_progress_revision=u32(data, mailbox["last_applied_progress_revision"]),
        status_flags=u32(data, mailbox["status_flags"]),
    )


def require_ready_mailbox(
    adapter: MemoryAdapter,
    address: int = MAILBOX_ADDRESS,
) -> MailboxHeader:
    try:
        header = read_header(adapter, address)
    except Exception as exc:
        raise MailboxReadinessError(
            "could not read the co-op mailbox at "
            f"0x{address:08X}: {exc}. "
            "Boot the verified co-op DOL in Dolphin, make sure this bridge is "
            "hooked to that Dolphin instance, and on macOS confirm Dolphin has "
            "the debug/memory-read entitlement after resigning."
        ) from exc
    if header.magic != proto.MAGIC:
        raise MailboxReadinessError(
            "co-op mailbox magic mismatch at "
            f"0x{address:08X}: got 0x{header.magic:08X}, expected "
            f"0x{proto.MAGIC:08X}. Boot the patched co-op DOL, not the "
            "stock game DOL."
        )
    if header.abi_version != proto.ABI_VERSION:
        raise MailboxReadinessError(
            "co-op mailbox ABI mismatch: "
            f"got {header.abi_version}, expected {proto.ABI_VERSION}. "
            "Rebuild the co-op DOL and bridge from the same checkout."
        )
    if header.struct_size != proto.MAILBOX_SIZE:
        raise MailboxReadinessError(
            "co-op mailbox size mismatch: "
            f"got {header.struct_size}, expected {proto.MAILBOX_SIZE}. "
            "Regenerate the protocol files and rebuild the co-op DOL."
        )
    if header.build_id != proto.BUILD_ID:
        raise MailboxReadinessError(
            "co-op mailbox build mismatch: "
            f"got 0x{header.build_id:08X}, expected 0x{proto.BUILD_ID:08X}. "
            "Rebuild the co-op DOL and bridge from the same checkout."
        )
    return header


def read_debug_state(adapter: MemoryAdapter, address: int = MAILBOX_ADDRESS) -> CoopDebugState:
    mailbox = proto.OFFSETS["CoopMailbox"]
    reserved = mailbox["reserved"]
    data = adapter.read(address + reserved, 32)
    values = struct.unpack(">8I", data)
    return CoopDebugState(
        magic=values[0],
        calls=values[1],
        reason=values[2],
        draws=values[3],
        hides=values[4],
        local_level=struct.unpack(">i", struct.pack(">I", values[5]))[0],
        inbound_level=struct.unpack(">i", struct.pack(">I", values[6]))[0],
        same_location=values[7],
    )


def read_snapshot_consistent(
    adapter: MemoryAdapter,
    seq_offset: int,
    snapshot_offset: int,
    address: int = MAILBOX_ADDRESS,
) -> bytes | None:
    seq_a = u32(adapter.read(address + seq_offset, 4), 0)
    if seq_a == 0 or seq_a & 1:
        return None
    snapshot = adapter.read(address + snapshot_offset, proto.SNAPSHOT_SIZE)
    seq_b = u32(adapter.read(address + seq_offset, 4), 0)
    if seq_a != seq_b or seq_b & 1:
        return None
    return snapshot


def read_local_snapshot_consistent(
    adapter: MemoryAdapter,
    address: int = MAILBOX_ADDRESS,
) -> bytes | None:
    mailbox = proto.OFFSETS["CoopMailbox"]
    return read_snapshot_consistent(
        adapter, mailbox["local_seq"], mailbox["local_snapshot"], address
    )


def write_inbound_snapshot(
    adapter: MemoryAdapter,
    snapshot: bytes,
    address: int = MAILBOX_ADDRESS,
) -> None:
    mailbox = proto.OFFSETS["CoopMailbox"]
    seq_offset = mailbox["inbound_seq"]
    snap_offset = mailbox["inbound_snapshot"]
    current = u32(adapter.read(address + seq_offset, 4), 0)
    if current & 1:
        current += 1
    adapter.write(address + seq_offset, struct.pack(">I", current + 1))
    adapter.write(address + snap_offset, snapshot[: proto.SNAPSHOT_SIZE])
    adapter.write(address + seq_offset, struct.pack(">I", current + 2))


def refresh_bridge_heartbeat(
    adapter: MemoryAdapter,
    address: int = MAILBOX_ADDRESS,
) -> int:
    mailbox = proto.OFFSETS["CoopMailbox"]
    heartbeat = read_header(adapter, address).game_heartbeat
    adapter.write(
        address + mailbox["bridge_heartbeat"],
        struct.pack(">I", heartbeat),
    )
    return heartbeat


def progress_from_snapshot(snapshot: bytes) -> dict[str, object]:
    snap = proto.OFFSETS["CoopSnapshot"]
    prog = proto.OFFSETS["CoopProgress"]
    base = snap["progress"]
    level_base = base + prog["level_flags"]
    hub_flags_base = base + prog["hub_flags"]
    hub_crystals_base = base + prog["hub_crystals"]
    return {
        "revision": u32(snapshot, base + prog["revision"]),
        "level_flags": [
            struct.unpack_from(">H", snapshot, level_base + i * 2)[0] for i in range(35)
        ],
        "hub_flags": list(snapshot[hub_flags_base : hub_flags_base + 6]),
        "hub_crystals": list(snapshot[hub_crystals_base : hub_crystals_base + 6]),
        "powerbits": snapshot[base + prog["powerbits"]],
        "gembits": snapshot[base + prog["gembits"]],
        "cutbits": u32(snapshot, base + prog["cutbits"]),
    }


def apply_progress_to_snapshot(snapshot: bytes, progress: dict[str, object]) -> bytes:
    data = bytearray(snapshot[: proto.SNAPSHOT_SIZE])
    snap = proto.OFFSETS["CoopSnapshot"]
    prog = proto.OFFSETS["CoopProgress"]
    base = snap["progress"]
    put_u32(data, base + prog["revision"], int(progress["revision"]))
    for i, value in enumerate(progress["level_flags"]):
        struct.pack_into(">H", data, base + prog["level_flags"] + i * 2, int(value) & 0xFFFF)
    for i, value in enumerate(progress["hub_flags"]):
        data[base + prog["hub_flags"] + i] = int(value) & 0xFF
    for i, value in enumerate(progress["hub_crystals"]):
        data[base + prog["hub_crystals"] + i] = int(value) & 0xFF
    data[base + prog["powerbits"]] = int(progress["powerbits"]) & 0xFF
    data[base + prog["gembits"]] = int(progress["gembits"]) & 0xFF
    put_u32(data, base + prog["cutbits"], int(progress.get("cutbits", 0)))
    return bytes(data)


def _with_status_and_avatar_flags(
    snapshot: bytes,
    status_flags: int,
    avatar_flags: int | None = None,
) -> bytes:
    data = bytearray(snapshot[: proto.SNAPSHOT_SIZE])
    snap = proto.OFFSETS["CoopSnapshot"]
    avatar = proto.OFFSETS["CoopAvatar"]
    struct.pack_into(">I", data, snap["status_flags"], status_flags)
    if avatar_flags is not None:
        struct.pack_into(">I", data, snap["avatar"] + avatar["flags"], avatar_flags)
    return bytes(data)


def make_active_remote_snapshot(
    remote_raw: bytes,
    progress: dict[str, object],
) -> bytes:
    snapshot = apply_progress_to_snapshot(remote_raw, progress)
    return _with_status_and_avatar_flags(
        snapshot,
        proto.STATUS_CONNECTED | proto.STATUS_ACTIVE,
        None,
    )


def make_connected_no_remote_snapshot(progress: dict[str, object]) -> bytes:
    snapshot = apply_progress_to_snapshot(bytes(proto.SNAPSHOT_SIZE), progress)
    return _with_status_and_avatar_flags(snapshot, proto.STATUS_CONNECTED, 0)


def make_disconnected_snapshot(progress: dict[str, object] | None = None) -> bytes:
    if progress is None:
        progress = {
            "revision": 0,
            "level_flags": [0] * 35,
            "hub_flags": [0] * 6,
            "hub_crystals": [0] * 6,
            "powerbits": 0,
            "gembits": 0,
            "cutbits": 0,
        }
    snapshot = apply_progress_to_snapshot(bytes(proto.SNAPSHOT_SIZE), progress)
    return _with_status_and_avatar_flags(snapshot, 0, 0)


class ProgressRevisionMapper:
    def __init__(self, last_applied_revision: int) -> None:
        if last_applied_revision < 0 or last_applied_revision > 0xFFFFFFFF:
            raise ValueError("last applied progress revision out of range")
        self._last_mapped = last_applied_revision
        self._last_content_key: tuple[Any, ...] | None = None
        self._last_progress: dict[str, object] | None = None
        self._server_revision_content: dict[int, tuple[Any, ...]] = {}
        self._pair_to_mapped: dict[tuple[int, tuple[Any, ...]], int] = {}

    @property
    def last_progress(self) -> dict[str, object] | None:
        return self._last_progress

    def map_progress(self, progress: dict[str, object]) -> dict[str, object]:
        server_revision = int(progress["revision"])
        if server_revision < 0 or server_revision > 0xFFFFFFFF:
            raise ValueError("server progress revision out of range")
        content_key = _progress_content_key(progress)
        previous_content = self._server_revision_content.get(server_revision)
        if previous_content is not None and previous_content != content_key:
            raise ValueError("server progress revision reused for different content")
        self._server_revision_content[server_revision] = content_key

        pair = (server_revision, content_key)
        mapped = self._pair_to_mapped.get(pair)
        if mapped is None:
            if self._last_content_key == content_key and self._last_progress is not None:
                mapped = self._last_mapped
            else:
                if self._last_mapped == 0xFFFFFFFF:
                    raise OverflowError("local progress delivery revision overflow")
                mapped = self._last_mapped + 1
            self._pair_to_mapped[pair] = mapped

        mapped_progress = _clone_progress(progress)
        mapped_progress["revision"] = mapped
        self._last_mapped = mapped
        self._last_content_key = content_key
        self._last_progress = mapped_progress
        return mapped_progress


def _progress_content_key(progress: dict[str, object]) -> tuple[Any, ...]:
    return (
        tuple(int(v) for v in progress["level_flags"]),  # type: ignore[index]
        tuple(int(v) for v in progress["hub_flags"]),  # type: ignore[index]
        tuple(int(v) for v in progress["hub_crystals"]),  # type: ignore[index]
        int(progress["powerbits"]),
        int(progress["gembits"]),
        int(progress.get("cutbits", 0)),
    )


def _clone_progress(progress: dict[str, object]) -> dict[str, object]:
    return {
        "revision": int(progress["revision"]),
        "level_flags": [int(v) for v in progress["level_flags"]],  # type: ignore[index]
        "hub_flags": [int(v) for v in progress["hub_flags"]],  # type: ignore[index]
        "hub_crystals": [int(v) for v in progress["hub_crystals"]],  # type: ignore[index]
        "powerbits": int(progress["powerbits"]),
        "gembits": int(progress["gembits"]),
        "cutbits": int(progress.get("cutbits", 0)),
    }


def force_different_location(snapshot: bytes) -> bytes:
    data = bytearray(snapshot[: proto.SNAPSHOT_SIZE])
    snap = proto.OFFSETS["CoopSnapshot"]
    loc = proto.OFFSETS["CoopLocation"]
    base = snap["location"]
    for offset in loc.values():
        struct.pack_into(">i", data, base + offset, DIFFERENT_LOCATION_SENTINEL)
    return bytes(data)


def snapshot_has_different_location(snapshot: bytes) -> bool:
    snap = proto.OFFSETS["CoopSnapshot"]
    loc = proto.OFFSETS["CoopLocation"]
    base = snap["location"]
    for offset in loc.values():
        if struct.unpack_from(">i", snapshot, base + offset)[0] != DIFFERENT_LOCATION_SENTINEL:
            return False
    return True


def read_inbound_snapshot_consistent(
    adapter: MemoryAdapter,
    address: int = MAILBOX_ADDRESS,
) -> bytes | None:
    mailbox = proto.OFFSETS["CoopMailbox"]
    return read_snapshot_consistent(
        adapter, mailbox["inbound_seq"], mailbox["inbound_snapshot"], address
    )


def ensure_hidden_inbound_snapshot(
    adapter: MemoryAdapter,
    offset_x: float = 0.0,
    address: int = MAILBOX_ADDRESS,
) -> bool:
    return mirror_local_snapshot(adapter, offset_x, True, address)


def mirror_local_snapshot(
    adapter: MemoryAdapter,
    offset_x: float = 0.0,
    different_level: bool = False,
    address: int = MAILBOX_ADDRESS,
) -> bool:
    mailbox = proto.OFFSETS["CoopMailbox"]
    local = read_snapshot_consistent(
        adapter, mailbox["local_seq"], mailbox["local_snapshot"], address
    )
    if local is None:
        return False
    snapshot = bytearray(local)
    snap = proto.OFFSETS["CoopSnapshot"]
    avatar = proto.OFFSETS["CoopAvatar"]
    struct.pack_into(">I", snapshot, snap["status_flags"], proto.STATUS_CONNECTED | proto.STATUS_ACTIVE)
    pos_x_offset = snap["avatar"] + avatar["pos_x"]
    struct.pack_into(">f", snapshot, pos_x_offset, struct.unpack_from(">f", snapshot, pos_x_offset)[0] + offset_x)
    if different_level:
        snapshot = bytearray(force_different_location(snapshot))
    write_inbound_snapshot(adapter, bytes(snapshot), address)
    refresh_bridge_heartbeat(adapter, address)
    return True
