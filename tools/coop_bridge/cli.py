from __future__ import annotations

import argparse
import asyncio
from contextlib import contextmanager
import os
from pathlib import Path
import struct
import sys
import time
import tempfile
from typing import Any

from . import protocol_generated as proto
from .memory import (
    DolphinMemoryAdapter,
    MAILBOX_ADDRESS,
    MemoryAdapter,
    apply_progress_to_snapshot,
    ensure_hidden_inbound_snapshot,
    mirror_local_snapshot,
    progress_from_snapshot,
    read_debug_state,
    read_header,
    read_inbound_snapshot_consistent,
    read_mailbox,
    refresh_bridge_heartbeat,
    write_inbound_snapshot,
)
from .net import read_message, write_message
from .session import SessionServer

DEBUG_REASONS = {
    1: "stale_bridge",
    2: "no_inbound",
    3: "inactive",
    4: "unused",
    5: "dead",
    6: "invisible",
    7: "local_state",
    8: "location_mismatch",
    9: "bad_float",
    10: "no_model",
    11: "bad_action",
    12: "drawn",
}


class BridgeWriterLockError(RuntimeError):
    pass


@contextmanager
def bridge_writer_lock(mailbox_address: int, command: str) -> Any:
    lock_path = Path(tempfile.gettempdir()) / f"crashwoc-coop-bridge-{mailbox_address:08X}.lock"
    lock_file = lock_path.open("a+b")
    acquired = False
    try:
        if lock_file.seek(0, os.SEEK_END) == 0:
            lock_file.write(b"\0")
            lock_file.flush()
        lock_file.seek(0)
        try:
            if os.name == "nt":
                import msvcrt

                msvcrt.locking(lock_file.fileno(), msvcrt.LK_NBLCK, 1)
            else:
                import fcntl

                fcntl.lockf(lock_file.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError as exc:
            raise BridgeWriterLockError(
                "another co-op bridge writer is already running for "
                f"mailbox 0x{mailbox_address:08X}; stop other inject-avatar/join "
                "processes first"
            ) from exc
        acquired = True
        lock_file.seek(0)
        lock_file.truncate()
        lock_file.write(f"pid={os.getpid()} command={command}\n".encode("ascii"))
        lock_file.flush()
        yield
    finally:
        if acquired:
            lock_file.seek(0)
            if os.name == "nt":
                import msvcrt

                msvcrt.locking(lock_file.fileno(), msvcrt.LK_UNLCK, 1)
            else:
                import fcntl

                fcntl.lockf(lock_file.fileno(), fcntl.LOCK_UN)
        lock_file.close()
        if acquired:
            try:
                lock_path.unlink()
            except OSError:
                pass


def make_adapter(fake: bool = False) -> MemoryAdapter:
    if fake:
        from .memory import FakeMemoryAdapter

        return FakeMemoryAdapter()
    return DolphinMemoryAdapter()


def diagnose(args: argparse.Namespace) -> int:
    adapter = make_adapter(args.fake)
    adapter.hook()
    header = read_header(adapter, args.mailbox)
    data = read_mailbox(adapter, args.mailbox)
    local = proto.OFFSETS["CoopMailbox"]["local_snapshot"]
    snap = proto.OFFSETS["CoopSnapshot"]
    loc = proto.OFFSETS["CoopLocation"]
    avatar = proto.OFFSETS["CoopAvatar"]
    level = struct.unpack_from(">i", data, local + snap["location"] + loc["level"])[0]
    pos_x = struct.unpack_from(">f", data, local + snap["avatar"] + avatar["pos_x"])[0]
    pos_y = struct.unpack_from(">f", data, local + snap["avatar"] + avatar["pos_y"])[0]
    pos_z = struct.unpack_from(">f", data, local + snap["avatar"] + avatar["pos_z"])[0]

    print(f"Dolphin hooked: {adapter.is_hooked()}")
    print(f"Mailbox magic: 0x{header.magic:08X}")
    print(f"ABI version: {header.abi_version}")
    print(f"Build ID: 0x{header.build_id:08X}")
    print(f"Game heartbeat: {header.game_heartbeat}")
    print(f"Current level: {level}")
    print(f"Local position: {pos_x:.2f}, {pos_y:.2f}, {pos_z:.2f}")
    if header.magic != proto.MAGIC:
        print("Compatibility: bad mailbox magic")
    elif header.abi_version != proto.ABI_VERSION:
        print("Compatibility: bad ABI")
    elif header.build_id != proto.BUILD_ID:
        print("Compatibility: bad build")
    else:
        print("Compatibility: ok")
    print_debug_state(adapter, args.mailbox)
    return 0


def print_debug_state(adapter: MemoryAdapter, mailbox_address: int) -> None:
    debug = read_debug_state(adapter, mailbox_address)
    reason = DEBUG_REASONS.get(debug.reason, f"unknown_{debug.reason}")
    magic = "ok" if debug.magic == 0x43444247 else f"0x{debug.magic:08X}"
    print(
        "Co-op debug: "
        f"magic={magic} calls={debug.calls} reason={reason} "
        f"draws={debug.draws} hides={debug.hides} "
        f"local_level={debug.local_level} inbound_level={debug.inbound_level} "
        f"same_location={debug.same_location}"
    )


def debug_log(args: argparse.Namespace) -> int:
    adapter = make_adapter(args.fake)
    adapter.hook()
    try:
        while True:
            header = read_header(adapter, args.mailbox)
            inbound = read_inbound_snapshot_consistent(adapter, args.mailbox)
            inbound_level = "none"
            if inbound is not None:
                snap = proto.OFFSETS["CoopSnapshot"]
                loc = proto.OFFSETS["CoopLocation"]
                inbound_level = str(
                    struct.unpack_from(">i", inbound, snap["location"] + loc["level"])[0]
                )
            print(
                f"heartbeat={header.game_heartbeat} bridge={header.bridge_heartbeat} "
                f"inbound_seq={header.inbound_seq} "
                f"mailbox_inbound_level={inbound_level} ",
                end="",
                flush=False,
            )
            print_debug_state(adapter, args.mailbox)
            if args.once:
                return 0
            time.sleep(args.interval)
    except KeyboardInterrupt:
        return 0


def inject_avatar(args: argparse.Namespace) -> int:
    with bridge_writer_lock(args.mailbox, "inject-avatar"):
        adapter = make_adapter(args.fake)
        adapter.hook()
        prime_until = 0.0
        if args.different_level and args.prime_visible > 0.0:
            prime_until = time.monotonic() + args.prime_visible
        try:
            while True:
                if args.different_level and time.monotonic() >= prime_until:
                    ensure_hidden_inbound_snapshot(adapter, args.offset_x, args.mailbox)
                else:
                    mirror_local_snapshot(adapter, args.offset_x, False, args.mailbox)
                time.sleep(0.05)
        except KeyboardInterrupt:
            return 0


async def host_async(args: argparse.Namespace) -> None:
    server = SessionServer(token=args.token, share_cutbits=args.share_cutbits)
    srv = await asyncio.start_server(server.handle_client, args.bind, args.port)
    async with srv:
        await srv.serve_forever()


async def join_async(args: argparse.Namespace) -> None:
    adapter = make_adapter(args.fake)
    adapter.hook()
    reader, writer = await asyncio.open_connection(args.host, args.port)
    await write_message(
        writer,
        {
            "type": "HELLO",
            "abi_version": proto.ABI_VERSION,
            "build_id": proto.BUILD_ID,
            "token": args.token,
        },
    )
    welcome = await read_message(reader)
    if welcome.get("type") != "WELCOME":
        raise RuntimeError(welcome)
    mailbox = proto.OFFSETS["CoopMailbox"]
    while True:
        data = read_mailbox(adapter, args.mailbox)
        snapshot = data[
            mailbox["local_snapshot"] : mailbox["local_snapshot"] + proto.SNAPSHOT_SIZE
        ]
        await write_message(
            writer,
            {
                "type": "LOCAL_SNAPSHOT",
                "snapshot": {
                    "raw": snapshot.hex(),
                    "progress": progress_from_snapshot(snapshot),
                },
            },
        )
        message = await read_message(reader)
        if message.get("type") == "SESSION_SNAPSHOT":
            snapshots = message.get("snapshots", {})
            for pid, remote in snapshots.items():
                if int(pid) == int(message.get("player_id", 0)):
                    continue
                raw = remote.get("raw") if isinstance(remote, dict) else None
                if isinstance(raw, str):
                    inbound = apply_progress_to_snapshot(bytes.fromhex(raw), message["progress"])
                    write_inbound_snapshot(adapter, inbound, args.mailbox)
                    break
            refresh_bridge_heartbeat(adapter, args.mailbox)
        await asyncio.sleep(0.05)


def join_command(args: argparse.Namespace) -> int:
    with bridge_writer_lock(args.mailbox, "join"):
        asyncio.run(join_async(args))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="python -m tools.coop_bridge")
    parser.add_argument("--mailbox", type=lambda s: int(s, 0), default=MAILBOX_ADDRESS)
    parser.add_argument("--fake", action="store_true", help=argparse.SUPPRESS)
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("diagnose").set_defaults(func=diagnose)

    debug = sub.add_parser("debug-log")
    debug.add_argument("--interval", type=float, default=0.5)
    debug.add_argument("--once", action="store_true")
    debug.set_defaults(func=debug_log)

    inject = sub.add_parser("inject-avatar")
    inject.add_argument("--offset-x", type=float, default=2.0)
    inject.add_argument("--different-level", action="store_true")
    inject.add_argument(
        "--prime-visible",
        type=float,
        default=0.0,
        help="with --different-level, first mirror a visible avatar for this many seconds",
    )
    inject.set_defaults(func=inject_avatar)

    host = sub.add_parser("host")
    host.add_argument("--bind", default="0.0.0.0")
    host.add_argument("--port", type=int, default=24827)
    host.add_argument("--token")
    host.add_argument("--share-cutbits", action="store_true")
    host.set_defaults(func=lambda args: asyncio.run(host_async(args)) or 0)

    join = sub.add_parser("join")
    join.add_argument("host")
    join.add_argument("--port", type=int, default=24827)
    join.add_argument("--token")
    join.set_defaults(func=join_command)
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return int(args.func(args))
    except BridgeWriterLockError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
