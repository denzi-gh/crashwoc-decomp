from __future__ import annotations

import asyncio
from dataclasses import dataclass, field
from typing import Any

from . import protocol_generated as proto
from .net import read_message, write_message


def empty_progress() -> dict[str, Any]:
    return {
        "revision": 0,
        "level_flags": [0] * 35,
        "hub_flags": [0] * 6,
        "hub_crystals": [0] * 6,
        "powerbits": 0,
        "gembits": 0,
        "cutbits": 0,
    }


def merge_progress(current: dict[str, Any], incoming: dict[str, Any], share_cutbits: bool = False) -> bool:
    changed = False
    for i in range(35):
        value = current["level_flags"][i] | incoming["level_flags"][i]
        changed |= value != current["level_flags"][i]
        current["level_flags"][i] = value
    for i in range(6):
        value = current["hub_flags"][i] | incoming["hub_flags"][i]
        changed |= value != current["hub_flags"][i]
        current["hub_flags"][i] = value
        crystals = max(current["hub_crystals"][i], incoming["hub_crystals"][i])
        changed |= crystals != current["hub_crystals"][i]
        current["hub_crystals"][i] = crystals
    for key in ("powerbits", "gembits"):
        value = current[key] | incoming[key]
        changed |= value != current[key]
        current[key] = value
    if share_cutbits:
        value = current["cutbits"] | incoming.get("cutbits", 0)
        changed |= value != current["cutbits"]
        current["cutbits"] = value
    if changed:
        current["revision"] += 1
    return changed


@dataclass
class Client:
    player_id: int
    writer: asyncio.StreamWriter
    snapshot: dict[str, Any] | None = None


@dataclass
class SessionServer:
    token: str | None = None
    share_cutbits: bool = False
    progress: dict[str, Any] = field(default_factory=empty_progress)
    clients: dict[int, Client] = field(default_factory=dict)
    next_player_id: int = 1

    async def handle_client(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
        player_id = 0
        try:
            hello = await read_message(reader)
            if hello.get("type") != "HELLO":
                await write_message(writer, {"type": "ERROR", "error": "expected HELLO"})
                return
            if hello.get("abi_version") != proto.ABI_VERSION:
                await write_message(writer, {"type": "ERROR", "error": "incompatible ABI"})
                return
            if hello.get("build_id") != proto.BUILD_ID:
                await write_message(writer, {"type": "ERROR", "error": "incompatible build"})
                return
            if self.token is not None and hello.get("token") != self.token:
                await write_message(writer, {"type": "ERROR", "error": "bad token"})
                return
            if len(self.clients) >= 2:
                await write_message(writer, {"type": "ERROR", "error": "session full"})
                return
            player_id = self.next_player_id
            self.next_player_id += 1
            self.clients[player_id] = Client(player_id, writer)
            await write_message(
                writer,
                {
                    "type": "WELCOME",
                    "player_id": player_id,
                    "progress": self.progress,
                },
            )
            while True:
                message = await read_message(reader)
                mtype = message.get("type")
                if mtype == "LOCAL_SNAPSHOT":
                    snapshot = message.get("snapshot", {})
                    self.clients[player_id].snapshot = snapshot
                    progress = snapshot.get("progress")
                    if isinstance(progress, dict):
                        merge_progress(self.progress, progress, self.share_cutbits)
                    await self.broadcast()
                elif mtype == "PING":
                    await write_message(writer, {"type": "PONG"})
                elif mtype == "DISCONNECT":
                    return
                else:
                    await write_message(writer, {"type": "ERROR", "error": "unknown message"})
        except (asyncio.IncompleteReadError, ConnectionError, TimeoutError, ValueError):
            return
        finally:
            if player_id:
                self.clients.pop(player_id, None)
            writer.close()
            await writer.wait_closed()

    async def broadcast(self) -> None:
        snapshots = {
            pid: client.snapshot
            for pid, client in self.clients.items()
            if client.snapshot is not None
        }
        dead: list[int] = []
        for pid, client in self.clients.items():
            try:
                await write_message(
                    client.writer,
                    {
                        "type": "SESSION_SNAPSHOT",
                        "player_id": pid,
                        "progress": self.progress,
                        "snapshots": snapshots,
                    },
                )
            except ConnectionError:
                dead.append(pid)
        for pid in dead:
            self.clients.pop(pid, None)

