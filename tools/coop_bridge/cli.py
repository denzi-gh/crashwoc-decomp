from __future__ import annotations

import argparse
import asyncio
import struct
import time
from typing import Any

from . import protocol_generated as proto
from .memory import (
    DolphinMemoryAdapter,
    MAILBOX_ADDRESS,
    MemoryAdapter,
    apply_progress_to_snapshot,
    mirror_local_snapshot,
    progress_from_snapshot,
    read_header,
    read_mailbox,
    write_inbound_snapshot,
)
from .net import read_message, write_message
from .session import SessionServer


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
    return 0


def inject_avatar(args: argparse.Namespace) -> int:
    adapter = make_adapter(args.fake)
    adapter.hook()
    try:
        while True:
            mirror_local_snapshot(adapter, args.offset_x, args.different_level, args.mailbox)
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
            hb = read_header(adapter, args.mailbox).game_heartbeat
            adapter.write(args.mailbox + mailbox["bridge_heartbeat"], struct.pack(">I", hb))
        await asyncio.sleep(0.05)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="python -m tools.coop_bridge")
    parser.add_argument("--mailbox", type=lambda s: int(s, 0), default=MAILBOX_ADDRESS)
    parser.add_argument("--fake", action="store_true", help=argparse.SUPPRESS)
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("diagnose").set_defaults(func=diagnose)

    inject = sub.add_parser("inject-avatar")
    inject.add_argument("--offset-x", type=float, default=2.0)
    inject.add_argument("--different-level", action="store_true")
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
    join.set_defaults(func=lambda args: asyncio.run(join_async(args)) or 0)
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return int(args.func(args))
